#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
bool network_init(int *fd,struct sockaddr_in *addr){
    *fd=socket(AF_INET,SOCK_STREAM,0); //init IPV4 and TCP connection
    if(*fd==-1){
        perror("socket initialize error");
        return 0; 
    }
    int opt=1;
    setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(bind(*fd,(struct sockaddr *)addr,sizeof(*addr))==-1){ //binding with localport
        perror("binding error");
        return 0;
    }
    if(listen(*fd,4096)==-1){
        perror("listen fail");
        return 0;
    }
    return 1;
}

