#include "parser.h"
#include <string.h>
#include <stddef.h>

#define IS_CRLF(str) (str == '\r' && str == '\n')

uint8_t parse_mailbox_line(buffer_t *buf, mailbox_t *box)
{
    char *tmp = strchr((buf->buf + buf->pos), '"');
    char *origin = NULL;

    while (*tmp != ' ')
        tmp++;

    if (*tmp == 0)
        return (1);

    *tmp = 0;

    origin = (tmp + 1);

    while (IS_CRLF(tmp) == 0)
        tmp++;

    *tmp = 0;

    if (*tmp == 0)
        return (1);

    box->name = origin;
    buf->pos += (tmp - buf->buf);

    return (0);
}

uint8_t parse_mailbox(buffer_t *buf, mailbox_t *box)
{
    return (0);
}
