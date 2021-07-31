#include "socket.h"
#include "parser.h"
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <unistd.h>

#define BUF_SIZE 8192

static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_entropy_context entropy;

int imap_connect(const char *host, uint16_t port)
{
    int fd;
    int res;
    char buf[BUF_SIZE];
    size_t len;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config ssl_conf;

    // Connect to server.
    if ((fd = socket_connect(host, port)) == -1)
        return (1);

    // Initialize SSL config.
    mbedtls_ssl_config_init(&ssl_conf);
    mbedtls_ssl_config_defaults(&ssl_conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_authmode(&ssl_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_rng(&ssl_conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    // Initialize SSL context.
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_setup(&ssl, &ssl_conf);
    mbedtls_ssl_set_bio(&ssl, &fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    // Do SSL handshake.
    if ((res = mbedtls_ssl_handshake(&ssl)))
        return (1);

    // On commence à ak
    mbedtls_ssl_read(&ssl, buf, BUF_SIZE);
    mbedtls_ssl_write(&ssl, "AKERPREAU001 LOGIN email password\r\n", 54);
    mbedtls_ssl_read(&ssl, buf, BUF_SIZE);
    mbedtls_ssl_write(&ssl, "AKERPREAU001 LIST \"\" *\r\n", 24);
    mbedtls_ssl_read(&ssl, buf, BUF_SIZE);
    mbedtls_ssl_write(&ssl, "AKERPREAU001 SELECT INBOX\r\n", 27);
    buf[mbedtls_ssl_read(&ssl, buf, BUF_SIZE)] = 0;
    fprintf(stderr, "%s", buf);
    mbedtls_ssl_write(&ssl, "AKERPREAU001 FETCH 468 (BODY.PEEK[])\r\n", 38);

    while (1) {
        len = mbedtls_ssl_read(&ssl, buf, BUF_SIZE);
        buf[len] = 0;
        fprintf(stderr, "%s", buf);
    }

    // Close file descriptor.
    close(fd);

    // Free SSL context.
    mbedtls_ssl_free(&ssl);

    return (0);
}

int main(int argc, char **argv)
{
    // Initialize entropy and random number generator.
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, "imap_client_for_aker_only", 25);

    // Connect to IMAP server.
    imap_connect(argv[1], 993);

    // Free random number generator and entropy.
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return (0);
}
