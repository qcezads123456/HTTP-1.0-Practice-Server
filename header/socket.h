#ifndef SOCKET_H
#define SOCKET_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
bool network_init(int *fd,struct sockaddr_in *addr); // error occured return 0 and error message else return 1

#endif