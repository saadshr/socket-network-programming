#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>

#include "ftp_common.h"

#define FTP_ADDRESS "127.0.0.1"

static void print_wsa_error(const char *operation)
{
    fprintf(stderr, "%s a échoué (Winsock %d).\n", operation, WSAGetLastError());
}

static void trim_newline(char *text)
{
    text[strcspn(text, "\r\n")] = '\0';
}

static int read_input(const char *prompt, char *buffer, size_t capacity)
{
    printf("%s", prompt);
    if (fgets(buffer, (int)capacity, stdin) == NULL) {
        return -1;
    }
    trim_newline(buffer);
    return 0;
}

static int read_response(SOCKET control, line_reader_t *reader, int *code_out)
{
    char line[FTP_CONTROL_BUFFER_SIZE];
    char expected_prefix[4] = { 0 };
    int code = 0;
    int first = 1;

    for (;;) {
        int received = recv_line(control, reader, line, sizeof(line));
        if (received <= 0) {
            return -1;
        }
        printf("<-- %s\n", line);
        if (first) {
            if (strlen(line) < 3 || sscanf(line, "%3d", &code) != 1) {
                return -1;
            }
            snprintf(expected_prefix, sizeof(expected_prefix), "%03d", code);
            first = 0;
            if (strlen(line) < 4 || line[3] != '-') {
                break;
            }
        } else if (strncmp(line, expected_prefix, 3) == 0 && line[3] == ' ') {
            break;
        }
    }
    *code_out = code;
    return 0;
}

static int send_command(SOCKET control, line_reader_t *reader,
                        const char *command, int *code_out)
{
    if (strncmp(command, "PASS ", 5) == 0) {
        printf("--> PASS [masqué]\n");
    } else {
        printf("--> %s\n", command);
    }
    if (ftp_send_line(control, command) != 0 || read_response(control, reader, code_out) != 0) {
        print_wsa_error("commande FTP");
        return -1;
    }
    return 0;
}

static int enter_passive_mode(SOCKET control, line_reader_t *reader, SOCKET *data_out)
{
    char response[FTP_CONTROL_BUFFER_SIZE];
    char address[64];
    unsigned short port;
    int code;

    printf("--> PASV\n");
    if (ftp_send_line(control, "PASV") != 0 ||
        recv_line(control, reader, response, sizeof(response)) <= 0) {
        return -1;
    }
    printf("<-- %s\n", response);
    if (sscanf(response, "%3d", &code) != 1 || code != 227 ||
        ftp_parse_pasv(response, address, sizeof(address), &port) != 0 ||
        ftp_connect_data(address, port, data_out) != 0) {
        return -1;
    }
    return 0;
}

static int receive_to_stdout(SOCKET data)
{
    char buffer[FTP_TRANSFER_BUFFER_SIZE];
    for (;;) {
        int received = recv(data, buffer, sizeof(buffer), 0);
        if (received == 0) {
            return 0;
        }
        if (received == SOCKET_ERROR) {
            return -1;
        }
        fwrite(buffer, 1, (size_t)received, stdout);
    }
}

static int receive_to_file(SOCKET data, const char *path)
{
    FILE *file = NULL;
    char buffer[FTP_TRANSFER_BUFFER_SIZE];
    if (fopen_s(&file, path, "wb") != 0 || file == NULL) {
        return -1;
    }
    for (;;) {
        int received = recv(data, buffer, sizeof(buffer), 0);
        if (received == 0) {
            fclose(file);
            return 0;
        }
        if (received == SOCKET_ERROR || fwrite(buffer, 1, (size_t)received, file) != (size_t)received) {
            fclose(file);
            return -1;
        }
    }
}

static int upload_file(SOCKET data, const char *path)
{
    FILE *file = NULL;
    char buffer[FTP_TRANSFER_BUFFER_SIZE];
    if (fopen_s(&file, path, "rb") != 0 || file == NULL) {
        return -1;
    }
    for (;;) {
        size_t count = fread(buffer, 1, sizeof(buffer), file);
        if (count > 0 && send_all(data, buffer, count) != 0) {
            fclose(file);
            return -1;
        }
        if (count < sizeof(buffer)) {
            if (ferror(file)) {
                fclose(file);
                return -1;
            }
            break;
        }
    }
    fclose(file);
    return shutdown(data, SD_SEND) == SOCKET_ERROR ? -1 : 0;
}

static int do_list(SOCKET control, line_reader_t *reader)
{
    char path[512];
    char command[FTP_CONTROL_BUFFER_SIZE];
    SOCKET data = INVALID_SOCKET;
    int code;
    if (read_input("Dossier virtuel (Entrée = /) : ", path, sizeof(path)) != 0) {
        return -1;
    }
    snprintf(command, sizeof(command), "%s%s", "LIST", *path ? " " : "");
    if (*path) {
        strncat_s(command, sizeof(command), path, _TRUNCATE);
    }
    if (enter_passive_mode(control, reader, &data) != 0 ||
        send_command(control, reader, command, &code) != 0 || code != 150) {
        if (data != INVALID_SOCKET) closesocket(data);
        return -1;
    }
    printf("--- contenu LIST ---\n");
    receive_to_stdout(data);
    closesocket(data);
    return read_response(control, reader, &code);
}

static int do_retr(SOCKET control, line_reader_t *reader)
{
    char remote[512];
    char local[512];
    char command[FTP_CONTROL_BUFFER_SIZE];
    SOCKET data = INVALID_SOCKET;
    int code;
    if (read_input("Fichier distant : ", remote, sizeof(remote)) != 0 ||
        read_input("Fichier local de sortie : ", local, sizeof(local)) != 0) {
        return -1;
    }
    snprintf(command, sizeof(command), "RETR %s", remote);
    if (enter_passive_mode(control, reader, &data) != 0 ||
        send_command(control, reader, command, &code) != 0 || code != 150) {
        if (data != INVALID_SOCKET) closesocket(data);
        return -1;
    }
    if (receive_to_file(data, local) != 0) {
        closesocket(data);
        return -1;
    }
    closesocket(data);
    return read_response(control, reader, &code);
}

static int do_stor(SOCKET control, line_reader_t *reader)
{
    char local[512];
    char remote[512];
    char command[FTP_CONTROL_BUFFER_SIZE];
    SOCKET data = INVALID_SOCKET;
    int code;
    if (read_input("Fichier local à envoyer : ", local, sizeof(local)) != 0 ||
        read_input("Fichier distant nouveau : ", remote, sizeof(remote)) != 0) {
        return -1;
    }
    snprintf(command, sizeof(command), "STOR %s", remote);
    if (enter_passive_mode(control, reader, &data) != 0 ||
        send_command(control, reader, command, &code) != 0 || code != 150) {
        if (data != INVALID_SOCKET) closesocket(data);
        return -1;
    }
    if (upload_file(data, local) != 0) {
        closesocket(data);
        return -1;
    }
    closesocket(data);
    return read_response(control, reader, &code);
}

static void print_menu(void)
{
    puts("\n=== SocketHub-C FTP ===");
    puts("1. SYST");
    puts("2. FEAT");
    puts("3. TYPE I");
    puts("4. PWD");
    puts("5. LIST");
    puts("6. RETR (télécharger)");
    puts("7. STOR (envoyer)");
    puts("8. NOOP");
    puts("9. QUIT");
}

int main(void)
{
    WSADATA wsa_data;
    SOCKET control = INVALID_SOCKET;
    struct sockaddr_in address;
    line_reader_t reader = { { 0 }, 0 };
    char input[64];
    char username[128];
    char password[128];
    int code;
    int status = 1;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        print_wsa_error("WSAStartup");
        return 1;
    }
    control = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (control == INVALID_SOCKET) {
        print_wsa_error("socket");
        goto cleanup;
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(FTP_ADDRESS);
    address.sin_port = htons(FTP_CONTROL_PORT);
    if (connect(control, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        print_wsa_error("connect");
        goto cleanup;
    }
    if (read_response(control, &reader, &code) != 0 || code != 220) {
        goto cleanup;
    }
    if (read_input("Utilisateur FTP : ", username, sizeof(username)) != 0 ||
        read_input("Mot de passe FTP : ", password, sizeof(password)) != 0) {
        goto cleanup;
    }
    {
        char command[FTP_CONTROL_BUFFER_SIZE];
        snprintf(command, sizeof(command), "USER %s", username);
        if (send_command(control, &reader, command, &code) != 0 || code != 331) goto cleanup;
        snprintf(command, sizeof(command), "PASS %s", password);
        if (send_command(control, &reader, command, &code) != 0 || code != 230) goto cleanup;
    }
    if (send_command(control, &reader, "TYPE I", &code) != 0 || code != 200) goto cleanup;

    for (;;) {
        print_menu();
        if (read_input("Choix : ", input, sizeof(input)) != 0) break;
        if (strcmp(input, "1") == 0) {
            send_command(control, &reader, "SYST", &code);
        } else if (strcmp(input, "2") == 0) {
            send_command(control, &reader, "FEAT", &code);
        } else if (strcmp(input, "3") == 0) {
            send_command(control, &reader, "TYPE I", &code);
        } else if (strcmp(input, "4") == 0) {
            send_command(control, &reader, "PWD", &code);
        } else if (strcmp(input, "5") == 0) {
            do_list(control, &reader);
        } else if (strcmp(input, "6") == 0) {
            do_retr(control, &reader);
        } else if (strcmp(input, "7") == 0) {
            do_stor(control, &reader);
        } else if (strcmp(input, "8") == 0) {
            send_command(control, &reader, "NOOP", &code);
        } else if (strcmp(input, "9") == 0) {
            if (send_command(control, &reader, "QUIT", &code) == 0 && code == 221) {
                status = 0;
            }
            break;
        } else {
            puts("Choix inconnu.");
        }
    }

cleanup:
    if (control != INVALID_SOCKET) {
        closesocket(control);
    }
    WSACleanup();
    return status;
}
