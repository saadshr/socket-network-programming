#include "ftp_common.h"

#include <stdio.h>
#include <string.h>

#include <windows.h>

static HANDLE open_utf8_file(const char *path, DWORD access, DWORD share_mode,
                             DWORD creation_disposition)
{
    WCHAR wide_path[32768];
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1,
                            wide_path, sizeof(wide_path) / sizeof(WCHAR)) == 0) {
        return INVALID_HANDLE_VALUE;
    }
    return CreateFileW(wide_path, access, share_mode, NULL,
                       creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
}

int ftp_send_line(SOCKET socket, const char *line)
{
    char framed[FTP_CONTROL_BUFFER_SIZE];
    int written = snprintf(framed, sizeof(framed), "%s\r\n", line);
    if (written < 0 || (size_t)written >= sizeof(framed)) {
        return -1;
    }
    return send_all(socket, framed, (size_t)written);
}

int ftp_parse_pasv(const char *response, char *address, size_t address_capacity,
                   unsigned short *port)
{
    int a, b, c, d, p1, p2;
    if (sscanf(response, "227%*[^(](%d,%d,%d,%d,%d,%d)",
               &a, &b, &c, &d, &p1, &p2) != 6 ||
        a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 ||
        d < 0 || d > 255 || p1 < 0 || p1 > 255 || p2 < 0 || p2 > 255) {
        return -1;
    }
    if (snprintf(address, address_capacity, "%d.%d.%d.%d", a, b, c, d) < 0) {
        return -1;
    }
    *port = (unsigned short)(p1 * 256 + p2);
    return 0;
}

int ftp_connect_data(const char *address, unsigned short port, SOCKET *socket_out)
{
    SOCKET socket_value = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in endpoint;

    if (socket_value == INVALID_SOCKET) {
        return -1;
    }
    memset(&endpoint, 0, sizeof(endpoint));
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = inet_addr(address);
    endpoint.sin_port = htons(port);

    if (connect(socket_value, (struct sockaddr *)&endpoint, sizeof(endpoint)) == SOCKET_ERROR) {
        closesocket(socket_value);
        return -1;
    }
    *socket_out = socket_value;
    return 0;
}

int ftp_send_file(SOCKET socket, const char *path)
{
    HANDLE file = open_utf8_file(path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
    char buffer[FTP_TRANSFER_BUFFER_SIZE];

    if (file == INVALID_HANDLE_VALUE) {
        return -1;
    }
    for (;;) {
        DWORD read_count = 0;
        DWORD offset = 0;
        if (!ReadFile(file, buffer, sizeof(buffer), &read_count, NULL)) {
            CloseHandle(file);
            return -1;
        }
        if (read_count == 0) {
            break;
        }
        while (offset < read_count) {
            int sent = send(socket, buffer + offset, (int)(read_count - offset), 0);
            if (sent == SOCKET_ERROR || sent == 0) {
                CloseHandle(file);
                return -1;
            }
            offset += (DWORD)sent;
        }
    }
    CloseHandle(file);
    return 0;
}

int ftp_receive_file(SOCKET socket, const char *path)
{
    HANDLE file = open_utf8_file(path, GENERIC_WRITE, 0, CREATE_NEW);
    char buffer[FTP_TRANSFER_BUFFER_SIZE];

    if (file == INVALID_HANDLE_VALUE) {
        return -1;
    }
    for (;;) {
        int received = recv(socket, buffer, sizeof(buffer), 0);
        if (received == 0) {
            CloseHandle(file);
            return 0;
        }
        if (received == SOCKET_ERROR) {
            CloseHandle(file);
            return -1;
        }
        {
            DWORD written_total = 0;
            while (written_total < (DWORD)received) {
                DWORD written = 0;
                if (!WriteFile(file, buffer + written_total,
                               (DWORD)received - written_total, &written, NULL) ||
                    written == 0) {
                    CloseHandle(file);
                    return -1;
                }
                written_total += written;
            }
        }
    }
}
