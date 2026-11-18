#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "net_io.h"

#define CHAT_ADDRESS "127.0.0.1"
#define CHAT_PORT 5000

static void print_wsa_error(const char *operation)
{
    fprintf(stderr, "%s a échoué (code Winsock %d).\n", operation, WSAGetLastError());
}

int main(void)
{
    WSADATA wsa_data;
    SOCKET connection = INVALID_SOCKET;
    struct sockaddr_in address;
    char message[NET_LINE_BUFFER_SIZE];
    line_reader_t reader = { { 0 }, 0 };
    int status = 1;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        print_wsa_error("WSAStartup");
        return 1;
    }

    connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connection == INVALID_SOCKET) {
        print_wsa_error("socket");
        goto cleanup;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(CHAT_ADDRESS);
    address.sin_port = htons(CHAT_PORT);

    if (connect(connection, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        print_wsa_error("connect");
        goto cleanup;
    }

    printf("Connecté à %s:%d. Tapez un message, ou /quit pour sortir.\n",
           CHAT_ADDRESS, CHAT_PORT);

    for (;;) {
        char response[NET_LINE_BUFFER_SIZE];

        printf("> ");
        if (fgets(message, sizeof(message), stdin) == NULL) {
            snprintf(message, sizeof(message), "/quit");
        }
        message[strcspn(message, "\r\n")] = '\0';
        if (strlen(message) >= NET_LINE_BUFFER_SIZE - 1) {
            fprintf(stderr, "Message trop long.\n");
            continue;
        }
        {
            size_t message_length = strlen(message);
            int should_quit = strcmp(message, "/quit") == 0;
            message[message_length] = '\n';
            message[message_length + 1] = '\0';

            if (send_all(connection, message, message_length + 1) != 0) {
                print_wsa_error("send");
                break;
            }

            {
                int received = recv_line(connection, &reader, response, sizeof(response));
                if (received == 0) {
                    printf("Le serveur s'est déconnecté.\n");
                    break;
                }
                if (received < 0) {
                    print_wsa_error("recv");
                    break;
                }
                printf("%s\n", response);
            }

            if (should_quit) {
                break;
            }
        }
    }

    status = 0;

cleanup:
    if (connection != INVALID_SOCKET) {
        closesocket(connection);
    }
    WSACleanup();
    return status;
}
