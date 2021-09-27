#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "socket.h"


///////////////////////////////////
//  INFO
///////////////////////////////////


int socket_get_host_addr(const char *host, struct sockaddr_in *addr)
{
    struct addrinfo hints = { 0 };
    struct addrinfo *res;

    // Set hint info.
    hints.ai_family     = AF_INET;          // IPV4 only.
    hints.ai_socktype   = SOCK_STREAM;      // Datagram socket.

    // Get address info.
    if (getaddrinfo(host, NULL, &hints, &res))
        return (-1);

    // Copy first address.
    memcpy(addr, res->ai_addr, res->ai_addrlen);

    // Free results.
    freeaddrinfo(res);

    return (0);
}


///////////////////////////////////
//  CONNECT
///////////////////////////////////


int socket_connect(const char *host, uint16_t port)
{
    int fd;
    struct sockaddr_in addr;

    // Create a socket.
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return(-1);

    // Set destination host.
    if (socket_get_host_addr(host, &addr) == -1)
        return (-1);

    // Set destination port.
    addr.sin_port = htons(port);

    // Connect to host.
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        return (-1);

    return (fd);
}
