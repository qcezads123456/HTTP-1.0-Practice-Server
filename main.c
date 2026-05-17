#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "parser.h"
#include "thread_pool.h"
#include "socket.h"

int main(){
    HTTP_t HT;
    memset(HT.req_str,0,sizeof(HT.req_str));
    int sockfd=0,new_socket;
    struct sockaddr_in addr = {0};
    addr.sin_addr.s_addr=INADDR_ANY; //init local address (0.0.0.0)
    addr.sin_family=AF_INET;
    addr.sin_port=htons(8080);
    const char html_dir[]="./.www/index.html";
    struct stat st;
    FILE *accpet_send_file;
    accpet_send_file=fopen(html_dir,"r");
    stat(html_dir, &st);
    if(!network_init(&sockfd,(&addr))){
        exit(EXIT_FAILURE);
    }
    while(1){
        socklen_t socklen=sizeof(addr);
        new_socket=accept(sockfd,(struct sockaddr*)&addr,&socklen);
        if(read(new_socket,HT.req_str,sizeof(HT.req_str))!=-1){
            int ret=sscanf(HT.req_str,"%19s %19s %19s",HT.req.method_str,HT.req.path_str,HT.req.version_str);
            printf("%d",ret);
            char *hello = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nSuccess!";
            write(new_socket,hello,strlen(hello));
            memset(HT.req_str, 0, sizeof(HT.req_str));
            close(new_socket);
        }

    }
    return 0;
}