#ifndef SOCKET_HUB_FTP_COMMON_H
#define SOCKET_HUB_FTP_COMMON_H

#include <stddef.h>
#include <winsock2.h>

#include "net_io.h"

#define FTP_CONTROL_PORT 2121
#define FTP_CONTROL_BUFFER_SIZE 8192
#define FTP_TRANSFER_BUFFER_SIZE 65536

int ftp_send_line(SOCKET socket, const char *line);
int ftp_parse_pasv(const char *response, char *address, size_t address_capacity,
                   unsigned short *port);
int ftp_connect_data(const char *address, unsigned short port, SOCKET *socket_out);
int ftp_send_file(SOCKET socket, const char *path);
int ftp_receive_file(SOCKET socket, const char *path);

#endif
