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
    SOCKET listener = INVALID_SOCKET;
    SOCKET client = INVALID_SOCKET;
    struct sockaddr_in address;
    char message[NET_LINE_BUFFER_SIZE];
    char response[NET_LINE_BUFFER_SIZE + 32];
    line_reader_t reader = { { 0 }, 0 };
    int status = 1;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        print_wsa_error("WSAStartup");
        return 1;
    }

    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) {
        print_wsa_error("socket");
        goto cleanup;
    }

    {
        BOOL reuse = TRUE;
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
                       (const char *)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
            print_wsa_error("setsockopt");
            goto cleanup;
        }
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(CHAT_ADDRESS);
    address.sin_port = htons(CHAT_PORT);

    if (bind(listener, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        print_wsa_error("bind");
        goto cleanup;
    }
    if (listen(listener, 1) == SOCKET_ERROR) {
        print_wsa_error("listen");
        goto cleanup;
    }

    printf("Serveur de chat en écoute sur %s:%d\n", CHAT_ADDRESS, CHAT_PORT);
    printf("Un client peut se connecter. Ctrl+C arrête le serveur.\n");

    client = accept(listener, NULL, NULL);
    if (client == INVALID_SOCKET) {
        print_wsa_error("accept");
        goto cleanup;
    }
    printf("Client connecté.\n");

    for (;;) {
        int received = recv_line(client, &reader, message, sizeof(message));
        if (received == 0) {
            printf("Le client s'est déconnecté.\n");
            break;
        }
        if (received < 0) {
            if (received == -1) {
                print_wsa_error("recv");
            } else {
                fprintf(stderr, "Message trop long (maximum %d octets).\n",
                        NET_LINE_BUFFER_SIZE - 1);
            }
            break;
        }

        printf("Client: %s\n", message);
        if (strcmp(message, "/quit") == 0) {
            const char goodbye[] = "Serveur: au revoir !\n";
            send_all(client, goodbye, sizeof(goodbye) - 1);
            break;
        }

        {
            int written = snprintf(response, sizeof(response),
                                   "Serveur: reçu -> %s\n", message);
            if (written < 0 || send_all(client, response, (size_t)written) != 0) {
                print_wsa_error("send");
                break;
            }
        }
    }

    status = 0;

cleanup:
    if (client != INVALID_SOCKET) {
        closesocket(client);
    }
    if (listener != INVALID_SOCKET) {
        closesocket(listener);
    }
    WSACleanup();
    return status;
}
