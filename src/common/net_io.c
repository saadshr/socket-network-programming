#include "net_io.h"

#include <string.h>

int send_all(SOCKET socket, const char *data, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        int result = send(socket, data + sent, (int)(length - sent), 0);
        if (result == SOCKET_ERROR || result == 0) {
            return -1;
        }
        sent += (size_t)result;
    }

    return 0;
}

int recv_line(SOCKET socket, line_reader_t *reader, char *line, size_t capacity)
{
    for (;;) {
        size_t i;

        for (i = 0; i < reader->used; ++i) {
            if (reader->data[i] == '\n') {
                size_t line_length = i;
                if (line_length > 0 && reader->data[line_length - 1] == '\r') {
                    --line_length;
                }
                if (line_length + 1 > capacity) {
                    return -2;
                }

                memcpy(line, reader->data, line_length);
                line[line_length] = '\0';
                ++i;
                memmove(reader->data, reader->data + i, reader->used - i);
                reader->used -= i;
                return 1;
            }
        }

        if (reader->used == sizeof(reader->data)) {
            return -2;
        }

        {
            int received = recv(socket, reader->data + reader->used,
                                 (int)(sizeof(reader->data) - reader->used), 0);
            if (received == 0) {
                return 0;
            }
            if (received == SOCKET_ERROR) {
                return -1;
            }
            reader->used += (size_t)received;
        }
    }
}
