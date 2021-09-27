#pragma once
#include <stdint.h>

typedef struct mailbox_t mailbox_t;
typedef struct buffer_t buffer_t;

struct mailbox_t
{
    mailbox_t *next;
    mailbox_t *prev;
    char *name;
};

struct buffer_t
{
    char *buf;
    uint64_t pos;
    uint64_t len;
};

uint8_t parse_mailbox_line(buffer_t *buf, mailbox_t *box);
uint8_t parse_mailbox(buffer_t *buf, mailbox_t *box);