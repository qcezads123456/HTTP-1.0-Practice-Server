#ifndef _PARSER_H
#define _PARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
typedef struct resp_use_t{
    int new_socket,sockfd;
}resp_use_t;
void respond_to_client(void *arg);
#endif 