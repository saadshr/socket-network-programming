#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <winsock2.h>
#include <windows.h>

#include "ftp_common.h"

#define FTP_ADDRESS "127.0.0.1"
#define FTP_ROOT_NAME L"ftp_root"
#define FTP_PATH_CAPACITY 32768
#define FTP_LINE_CAPACITY 8192

typedef struct {
    WCHAR root[FTP_PATH_CAPACITY];
    size_t root_length;
} ftp_sandbox_t;

static void print_wsa_error(const char *operation)
{
    fprintf(stderr, "%s a échoué (Winsock %d).\n", operation, WSAGetLastError());
}

static int send_reply(SOCKET control, const char *reply)
{
    printf("S -> C: %s\n", reply);
    return ftp_send_line(control, reply);
}

static int utf8_to_wide(const char *input, WCHAR *output, size_t capacity)
{
    int converted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, -1,
                                        output, (int)capacity);
    return converted > 0 ? 0 : -1;
}

static int sandbox_init(ftp_sandbox_t *sandbox)
{
    DWORD length;
    if (!CreateDirectoryW(FTP_ROOT_NAME, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return -1;
    }
    length = GetFullPathNameW(FTP_ROOT_NAME, (DWORD)sizeof(sandbox->root) / sizeof(WCHAR),
                              sandbox->root, NULL);
    if (length == 0 || length >= sizeof(sandbox->root) / sizeof(WCHAR)) {
        return -1;
    }
    if ((GetFileAttributesW(sandbox->root) & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
        fprintf(stderr, "Le dossier FTP ne doit pas être un reparse point.\n");
        return -1;
    }
    sandbox->root_length = wcslen(sandbox->root);
    return 0;
}

static int append_component(WCHAR *path, size_t capacity, const WCHAR *component)
{
    if (wcscat_s(path, capacity, L"\\") != 0 || wcscat_s(path, capacity, component) != 0) {
        return -1;
    }
    return 0;
}

/*
 * Résout uniquement un chemin virtuel relatif au dossier FTP.
 * Chaque composant existant est contrôlé contre FILE_ATTRIBUTE_REPARSE_POINT.
 */
static int sandbox_resolve(const ftp_sandbox_t *sandbox, const char *virtual_path,
                           WCHAR *resolved, size_t capacity, int require_existing,
                           int require_directory, int allow_missing_final)
{
    WCHAR relative[FTP_PATH_CAPACITY];
    WCHAR current[FTP_PATH_CAPACITY];
    WCHAR *cursor;
    WCHAR *component;
    int component_index = 0;

    if (virtual_path == NULL || utf8_to_wide(virtual_path, relative,
                                              sizeof(relative) / sizeof(WCHAR)) != 0) {
        return -1;
    }
    if (relative[0] == L'\\' || relative[0] == L'/' || wcschr(relative, L':') != NULL) {
        return -1;
    }
    for (cursor = relative; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'/') {
            *cursor = L'\\';
        }
    }
    if (relative[0] == L'\0' || wcscmp(relative, L".") == 0) {
        relative[0] = L'\0';
    }

    if (wcsncpy_s(resolved, capacity, sandbox->root, _TRUNCATE) != 0 ||
        wcsncpy_s(current, sizeof(current) / sizeof(WCHAR), sandbox->root, _TRUNCATE) != 0) {
        return -1;
    }
    cursor = relative;
    while (*cursor != L'\0') {
        WCHAR *end = wcschr(cursor, L'\\');
        int is_last = end == NULL;
        if (end != NULL) {
            *end = L'\0';
        }
        component = cursor;
        if (component[0] == L'\0' || wcscmp(component, L".") == 0 ||
            wcscmp(component, L"..") == 0) {
            return -1;
        }
        if (append_component(current, sizeof(current) / sizeof(WCHAR), component) != 0 ||
            append_component(resolved, capacity, component) != 0) {
            return -1;
        }
        {
            DWORD attributes = GetFileAttributesW(current);
            if (attributes == INVALID_FILE_ATTRIBUTES) {
                if (!(is_last && allow_missing_final)) {
                    return -1;
                }
            } else {
                if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0 ||
                    (!is_last && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)) {
                    return -1;
                }
                if (is_last && require_directory &&
                    (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    return -1;
                }
            }
        }
        ++component_index;
        if (is_last) {
            break;
        }
        cursor = end + 1;
    }
    {
        DWORD root_attributes = GetFileAttributesW(sandbox->root);
        DWORD final_attributes = GetFileAttributesW(resolved);
        if (root_attributes == INVALID_FILE_ATTRIBUTES ||
            (root_attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
            return -1;
        }
        if (require_existing && final_attributes == INVALID_FILE_ATTRIBUTES) {
            return -1;
        }
        if (require_directory && final_attributes != INVALID_FILE_ATTRIBUTES &&
            (final_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            return -1;
        }
    }
    (void)component_index;
    return 0;
}

static int create_listener(unsigned short port, SOCKET *listener, unsigned short *actual_port)
{
    SOCKET value = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in address;
    int address_length = sizeof(address);

    if (value == INVALID_SOCKET) {
        return -1;
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(FTP_ADDRESS);
    address.sin_port = htons(port);
    if (bind(value, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR ||
        listen(value, 1) == SOCKET_ERROR ||
        getsockname(value, (struct sockaddr *)&address, &address_length) == SOCKET_ERROR) {
        closesocket(value);
        return -1;
    }
    *listener = value;
    *actual_port = ntohs(address.sin_port);
    return 0;
}

static SOCKET accept_data(SOCKET listener)
{
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(listener, &set);
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (select(0, &set, NULL, NULL, &timeout) <= 0) {
        return INVALID_SOCKET;
    }
    return accept(listener, NULL, NULL);
}

static int send_listing(SOCKET data, const WCHAR *directory)
{
    WCHAR pattern[FTP_PATH_CAPACITY];
    WIN32_FIND_DATAW item;
    HANDLE search;
    if (wcsncpy_s(pattern, sizeof(pattern) / sizeof(WCHAR), directory, _TRUNCATE) != 0 ||
        wcscat_s(pattern, sizeof(pattern) / sizeof(WCHAR), L"\\*") != 0) {
        return -1;
    }
    search = FindFirstFileW(pattern, &item);
    if (search == INVALID_HANDLE_VALUE) {
        return -1;
    }
    do {
        char name[FTP_PATH_CAPACITY];
        char line[FTP_LINE_CAPACITY];
        ULARGE_INTEGER size;
        int name_length;
        if (wcscmp(item.cFileName, L".") == 0 || wcscmp(item.cFileName, L"..") == 0 ||
            (item.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
            continue;
        }
        name_length = WideCharToMultiByte(CP_UTF8, 0, item.cFileName, -1,
                                          name, sizeof(name), NULL, NULL);
        if (name_length == 0) {
            FindClose(search);
            return -1;
        }
        size.LowPart = item.nFileSizeLow;
        size.HighPart = item.nFileSizeHigh;
        if (snprintf(line, sizeof(line), "%c %llu %s\r\n",
                     (item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 'd' : '-',
                     (unsigned long long)size.QuadPart, name) < 0 ||
            send_all(data, line, strlen(line)) != 0) {
            FindClose(search);
            return -1;
        }
    } while (FindNextFileW(search, &item));
    FindClose(search);
    return 0;
}

static int handle_session(SOCKET control, const ftp_sandbox_t *sandbox,
                           const char *configured_user, const char *configured_password)
{
    line_reader_t reader = { { 0 }, 0 };
    char line[FTP_LINE_CAPACITY];
    char username[128] = { 0 };
    int username_valid = 0;
    int authenticated = 0;
    SOCKET passive_listener = INVALID_SOCKET;

    if (send_reply(control, "220 SocketHub-C FTP ready") != 0) {
        return -1;
    }
    for (;;) {
        char command[16] = { 0 };
        char *argument;
        int received = recv_line(control, &reader, line, sizeof(line));
        if (received <= 0) {
            break;
        }
        argument = line;
        while (*argument == ' ' || *argument == '\t') {
            ++argument;
        }
        if (sscanf(argument, "%15s", command) != 1) {
            send_reply(control, "500 Empty command");
            continue;
        }
        argument += strlen(command);
        while (*argument == ' ' || *argument == '\t') {
            ++argument;
        }
        for (char *p = command; *p != '\0'; ++p) {
            if (*p >= 'a' && *p <= 'z') {
                *p = (char)(*p - 'a' + 'A');
            }
        }
        printf("C -> S: %s %s\n", command, argument);

        if (strcmp(command, "USER") == 0) {
            strncpy_s(username, sizeof(username), argument, _TRUNCATE);
            username_valid = _stricmp(username, configured_user) == 0;
            authenticated = 0;
            send_reply(control, "331 User name okay, need password");
        } else if (strcmp(command, "PASS") == 0) {
            if (username_valid && strcmp(argument, configured_password) == 0) {
                authenticated = 1;
                send_reply(control, "230 User logged in, proceed");
            } else {
                send_reply(control, "530 Not logged in");
            }
        } else if (strcmp(command, "SYST") == 0) {
            send_reply(control, "215 UNIX Type: L8");
        } else if (strcmp(command, "FEAT") == 0) {
            send_reply(control, "211-Features:");
            send_reply(control, " PASV");
            send_reply(control, " EPSV");
            send_reply(control, " UTF8");
            send_reply(control, "211 End");
        } else if (strcmp(command, "NOOP") == 0) {
            send_reply(control, "200 NOOP ok");
        } else if (strcmp(command, "QUIT") == 0) {
            send_reply(control, "221 Service closing control connection");
            break;
        } else if (!authenticated) {
            send_reply(control, "530 Not logged in");
        } else if (strcmp(command, "TYPE") == 0) {
            if (_stricmp(argument, "I") == 0 || _stricmp(argument, "A") == 0) {
                send_reply(control, _stricmp(argument, "I") == 0
                                      ? "200 Type set to I"
                                      : "200 Type set to A");
            } else {
                send_reply(control, "504 Only binary type I is supported");
            }
        } else if (strcmp(command, "PWD") == 0) {
            send_reply(control, "257 \"/\" is the current directory");
        } else if (strcmp(command, "PASV") == 0 || strcmp(command, "EPSV") == 0) {
            unsigned short port;
            if (passive_listener != INVALID_SOCKET) {
                closesocket(passive_listener);
                passive_listener = INVALID_SOCKET;
            }
            if (create_listener(0, &passive_listener, &port) != 0) {
                send_reply(control, "425 Can't open passive connection");
            } else {
                char reply[128];
                int p1 = port / 256;
                int p2 = port % 256;
                if (strcmp(command, "EPSV") == 0) {
                    snprintf(reply, sizeof(reply), "229 Entering Extended Passive Mode (|||%u|)", port);
                } else {
                    snprintf(reply, sizeof(reply),
                             "227 Entering Passive Mode (127,0,0,1,%d,%d)", p1, p2);
                }
                send_reply(control, reply);
            }
        } else if (strcmp(command, "LIST") == 0) {
            WCHAR directory[FTP_PATH_CAPACITY];
            SOCKET data;
            if (sandbox_resolve(sandbox, *argument ? argument : ".", directory,
                                sizeof(directory) / sizeof(WCHAR), 1, 1, 0) != 0) {
                send_reply(control, "550 Invalid or inaccessible path");
                continue;
            }
            if (passive_listener == INVALID_SOCKET) {
                send_reply(control, "425 Use PASV first");
                continue;
            }
            send_reply(control, "150 Opening binary mode data connection for LIST");
            data = accept_data(passive_listener);
            closesocket(passive_listener);
            passive_listener = INVALID_SOCKET;
            if (data == INVALID_SOCKET) {
                send_reply(control, "425 Can't open data connection");
            } else if (send_listing(data, directory) != 0) {
                closesocket(data);
                send_reply(control, "426 Connection closed; transfer aborted");
            } else {
                closesocket(data);
                send_reply(control, "226 Transfer complete");
            }
        } else if (strcmp(command, "RETR") == 0) {
            WCHAR path[FTP_PATH_CAPACITY];
            char narrow_path[FTP_PATH_CAPACITY];
            SOCKET data;
            if (!*argument || sandbox_resolve(sandbox, argument, path,
                                               sizeof(path) / sizeof(WCHAR), 1, 0, 0) != 0 ||
                (GetFileAttributesW(path) & FILE_ATTRIBUTE_DIRECTORY) != 0 ||
                WideCharToMultiByte(CP_UTF8, 0, path, -1, narrow_path,
                                    sizeof(narrow_path), NULL, NULL) == 0) {
                send_reply(control, "550 File unavailable");
                continue;
            }
            if (passive_listener == INVALID_SOCKET) {
                send_reply(control, "425 Use PASV first");
                continue;
            }
            send_reply(control, "150 Opening binary mode data connection for RETR");
            data = accept_data(passive_listener);
            closesocket(passive_listener);
            passive_listener = INVALID_SOCKET;
            if (data == INVALID_SOCKET) {
                send_reply(control, "425 Can't open data connection");
            } else if (ftp_send_file(data, narrow_path) != 0) {
                closesocket(data);
                send_reply(control, "426 Connection closed; transfer aborted");
            } else {
                closesocket(data);
                send_reply(control, "226 Transfer complete");
            }
        } else if (strcmp(command, "STOR") == 0) {
            WCHAR path[FTP_PATH_CAPACITY];
            WCHAR parent[FTP_PATH_CAPACITY];
            WCHAR temp[FTP_PATH_CAPACITY];
            char temp_narrow[FTP_PATH_CAPACITY];
            char final_narrow[FTP_PATH_CAPACITY];
            WCHAR *separator;
            SOCKET data;
            DWORD attributes;
            if (!*argument || sandbox_resolve(sandbox, argument, path,
                                               sizeof(path) / sizeof(WCHAR), 0, 0, 1) != 0) {
                send_reply(control, "550 Invalid destination");
                continue;
            }
            attributes = GetFileAttributesW(path);
            if (attributes != INVALID_FILE_ATTRIBUTES) {
                send_reply(control, "550 File already exists");
                continue;
            }
            if (wcsncpy_s(parent, sizeof(parent) / sizeof(WCHAR), path, _TRUNCATE) != 0 ||
                (separator = wcsrchr(parent, L'\\')) == NULL) {
                send_reply(control, "550 Invalid destination");
                continue;
            }
            *separator = L'\0';
            if (GetFileAttributesW(parent) == INVALID_FILE_ATTRIBUTES ||
                (GetFileAttributesW(parent) & FILE_ATTRIBUTE_DIRECTORY) == 0 ||
                (GetFileAttributesW(parent) & FILE_ATTRIBUTE_REPARSE_POINT) != 0 ||
                GetTempFileNameW(parent, L"sh", 0, temp) == 0 ||
                WideCharToMultiByte(CP_UTF8, 0, temp, -1, temp_narrow,
                                    sizeof(temp_narrow), NULL, NULL) == 0 ||
                WideCharToMultiByte(CP_UTF8, 0, path, -1, final_narrow,
                                    sizeof(final_narrow), NULL, NULL) == 0) {
                send_reply(control, "550 Cannot prepare destination");
                continue;
            }
            DeleteFileW(temp);
            if (passive_listener == INVALID_SOCKET) {
                send_reply(control, "425 Use PASV first");
                continue;
            }
            send_reply(control, "150 Opening binary mode data connection for STOR");
            data = accept_data(passive_listener);
            closesocket(passive_listener);
            passive_listener = INVALID_SOCKET;
            if (data == INVALID_SOCKET) {
                DeleteFileW(temp);
                send_reply(control, "425 Can't open data connection");
            } else if (ftp_receive_file(data, temp_narrow) != 0) {
                closesocket(data);
                DeleteFileW(temp);
                send_reply(control, "426 Connection closed; transfer aborted");
            } else {
                closesocket(data);
                if (!MoveFileExW(temp, path, MOVEFILE_COPY_ALLOWED)) {
                    DeleteFileW(temp);
                    send_reply(control, "550 Cannot commit uploaded file");
                } else {
                    send_reply(control, "226 Transfer complete");
                }
            }
        } else {
            send_reply(control, "502 Command not implemented");
        }
    }
    if (passive_listener != INVALID_SOCKET) {
        closesocket(passive_listener);
    }
    return 0;
}

int main(void)
{
    WSADATA wsa_data;
    SOCKET listener = INVALID_SOCKET;
    ftp_sandbox_t sandbox;
    const char *configured_user = getenv("SOCKETHUB_FTP_USER");
    const char *configured_password = getenv("SOCKETHUB_FTP_PASSWORD");
    struct sockaddr_in address;
    int status = 1;

    if (configured_user == NULL || *configured_user == '\0') {
        configured_user = "student";
    }
    if (configured_password == NULL) {
        fprintf(stderr, "Définissez SOCKETHUB_FTP_PASSWORD avant de lancer le serveur.\n");
        return 1;
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        print_wsa_error("WSAStartup");
        return 1;
    }
    if (sandbox_init(&sandbox) != 0) {
        fprintf(stderr, "Impossible d'initialiser le dossier ftp_root.\n");
        goto cleanup;
    }
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) {
        print_wsa_error("socket");
        goto cleanup;
    }
    {
        BOOL reuse = TRUE;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(FTP_ADDRESS);
    address.sin_port = htons(FTP_CONTROL_PORT);
    if (bind(listener, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR ||
        listen(listener, 4) == SOCKET_ERROR) {
        print_wsa_error("bind/listen");
        goto cleanup;
    }
    printf("Serveur FTP SocketHub-C sur %s:%d\n", FTP_ADDRESS, FTP_CONTROL_PORT);
    printf("Dossier autorisé : ftp_root ; utilisateur : %s\n", configured_user);
    for (;;) {
        SOCKET control = accept(listener, NULL, NULL);
        if (control == INVALID_SOCKET) {
            print_wsa_error("accept");
            goto cleanup;
        }
        handle_session(control, &sandbox, configured_user, configured_password);
        closesocket(control);
    }
    status = 0;

cleanup:
    if (listener != INVALID_SOCKET) {
        closesocket(listener);
    }
    WSACleanup();
    return status;
}
