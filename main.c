#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include "parser.h"
#include "thread_pool.h"
#include "socket.h"
volatile sig_atomic_t stop=0;
void handle_sigint(int sig){
    printf("caught signal :%d\n",sig);
    stop=1;
}

// signal bug left

int main(){
    int count=1;
    resp_use_t *res;
    thread_pool_t *pool=pool_init();
    res=(resp_use_t*)malloc(sizeof(resp_use_t));
    struct sockaddr_in addr={0};
    addr=addr_init();
    if(!network_init(&res->sockfd,(&addr))){
        pool_destroy(pool);
        exit(EXIT_FAILURE);
    }
    socklen_t socklen=sizeof(addr);
    signal(SIGINT,handle_sigint);
    while(!stop){
        resp_use_t *client_res;
        client_res=(resp_use_t*)malloc(sizeof(resp_use_t));
        client_res->sockfd=res->sockfd;
        res->new_socket=accept(res->sockfd,(struct sockaddr*)&addr,&socklen);
        client_res->new_socket=res->new_socket;
        if(addTask(respond_to_client,client_res)){
            printf("\nNEXT %d",count++);

        }
        else{
            char *res_msg=
                "HTTP/1.0 503 Service Unvailable\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "Service Busy";
            send(client_res->new_socket,res_msg,strlen(res_msg),0);
            close(client_res->new_socket);
            free(client_res);
        }

    }
    pool->shutdown=1;
    pool_destroy(pool);
    return 0;
}



