#include "socket.h"
#include "parser.h"
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 8192


///////////////////////////////////
//  STRUCTURE
///////////////////////////////////


typedef struct
{
    int                     fd;
    mbedtls_ssl_context     ssl_ctx;
    mbedtls_ssl_config      ssl_conf;
    buffer_t                buffer;
} imap_client_t;

static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_entropy_context  entropy;


///////////////////////////////////
//  IMAP
///////////////////////////////////


int imap_connect(imap_client_t *client, const char *host, uint16_t port)
{
    // Connect to server.
    if ((client->fd = socket_connect(host, port)) == -1)
        return (1);

    // Initialize SSL config.
    mbedtls_ssl_config_init(&client->ssl_conf);
    mbedtls_ssl_config_defaults(&client->ssl_conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_authmode(&client->ssl_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_rng(&client->ssl_conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    // Initialize SSL context.
    mbedtls_ssl_init(&client->ssl_ctx);
    mbedtls_ssl_setup(&client->ssl_ctx, &client->ssl_conf);
    mbedtls_ssl_set_bio(&client->ssl_ctx, &client->fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    // Perform SSL handshake.
    if (mbedtls_ssl_handshake(&client->ssl_ctx))
        return (1);

    return (0);
}

void imap_close(imap_client_t *client)
{
    // Close file descriptor.
    close(client->fd);

    // Free SSL context.
    mbedtls_ssl_free(&client->ssl_ctx);
}

int imap_login(imap_client_t *client, const char *email, const char *password)
{
    char buf[2048];
    char answer[256];
    size_t len;

    // Read and set null terminated string.
    len         = mbedtls_ssl_read(&client->ssl_ctx, buf, 2048);
    buf[len]    = 0;

    // Build and send LOGIN command.
    len = snprintf(answer, 256, "A001 LOGIN %s %s\r\n", email, password);
    mbedtls_ssl_write(&client->ssl_ctx, answer, len);

    // Read and set null terminated string.
    len         = mbedtls_ssl_read(&client->ssl_ctx, buf, 2048);
    buf[len]    = 0;

    if ((strstr(buf, "OK") - buf) != 5)
        return (1);

    return (0);
}

int imap_list_mailboxes(imap_client_t *client)
{
    char buf[2048];
    size_t len;

    // Send LIST command.
    mbedtls_ssl_write(&client->ssl_ctx, "A002 LIST \"\" *\r\n", 16);

    // Read and set null terminated string.
    len         = mbedtls_ssl_read(&client->ssl_ctx, buf, 2048);
    buf[len]    = 0;

    fprintf(stderr, "%s\n", buf);

    if ((strstr(buf, "OK") - buf) != 5)
        return (1);

    return (0);
}

int imap_dump_mails(imap_client_t *client)
{
    // On commence à ak
    // mbedtls_ssl_read(&client->ssl_ctx, client->buffer.buf, BUF_SIZE);
    mbedtls_ssl_write(&client->ssl_ctx, "A002 LIST \"\" *\r\n", 24);
    mbedtls_ssl_read(&client->ssl_ctx, client->buffer.buf, BUF_SIZE);

    // parser

    mbedtls_ssl_write(&client->ssl_ctx, "A003 SELECT INBOX\r\n", 27);
    client->buffer.buf[mbedtls_ssl_read(&client->ssl_ctx, client->buffer.buf, BUF_SIZE)] = 0;
    fprintf(stderr, "%s", client->buffer.buf);
    mbedtls_ssl_write(&client->ssl_ctx, "A004 FETCH 468 (BODY.PEEK[])\r\n", 38);

    while (1) {
        client->buffer.len = mbedtls_ssl_read(&client->ssl_ctx, client->buffer.buf, BUF_SIZE);
        client->buffer.buf[client->buffer.len] = 0;
        fprintf(stderr, "%s", client->buffer.buf);
    }
}


///////////////////////////////////
//  IMAP
///////////////////////////////////


int main(int argc, char **argv)
{
    imap_client_t client;

    // Initialize entropy and random number generator.
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, "imap_client_for_aker_only", 25);

    // Connect to IMAP server.
    if (imap_connect(&client, argv[1], 993))
        return (1);

    // Login into account.
    if (imap_login(&client, argv[2], argv[3]))
        return (1);

    // List account mailboxes.
    if (imap_list_mailboxes(&client))
        return (1);

    printf("ez?\n");

    // Close connection.
    imap_close(&client);

    // Free random number generator and entropy.
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return (0);
}
