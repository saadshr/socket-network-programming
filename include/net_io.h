#ifndef SOCKET_HUB_NET_IO_H
#define SOCKET_HUB_NET_IO_H

#include <stddef.h>
#include <winsock2.h>

#define NET_LINE_BUFFER_SIZE 4096

typedef struct {
    char data[NET_LINE_BUFFER_SIZE];
    size_t used;
} line_reader_t;

/* Envoie exactement length octets, même si send() réalise un envoi partiel. */
int send_all(SOCKET socket, const char *data, size_t length);

/*
 * Lit une ligne terminée par '\n' depuis un flux TCP.
 * Retourne 1 pour une ligne, 0 pour une déconnexion propre,
 * -1 pour une erreur Winsock et -2 pour une ligne trop longue.
 */
int recv_line(SOCKET socket, line_reader_t *reader, char *line, size_t capacity);

#endif
