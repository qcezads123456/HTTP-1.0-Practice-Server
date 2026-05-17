#ifndef _PARSER_H
#define _PARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

typedef enum{
    GET,
    UNKNOWN,
}method;

typedef struct {
    char method_str[20];
    char path_str[20];
    char version_str[20];
}request_line_t;

typedef struct{
    char req_str[4096];
    request_line_t req;
}HTTP_t;

#endif 