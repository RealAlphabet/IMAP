#pragma once

#include <stdint.h>
#include <arpa/inet.h>

///////////////////////////////////
//  SOCKET
///////////////////////////////////

// socket.c
int socket_get_host_addr(const char *host, struct sockaddr_in *addr);
int socket_connect(const char *host, uint16_t port);