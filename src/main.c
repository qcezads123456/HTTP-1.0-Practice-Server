#define _GNU_SOURCE
#include "main.h"

volatile sig_atomic_t stop=0;
void handle_sigint(int sig){
    stop=1;
}

// signal bug left
int main(){
    //int count=1;
    signal(SIGPIPE,SIG_IGN); // ignore the SIGPIPE when client close the connection or timeout
    resp_use_t *res;
    thread_pool_t *pool=pool_init();
    res=(resp_use_t*)malloc(sizeof(resp_use_t));
    struct sockaddr_in addr={0};
    addr=addr_init();
    struct sigaction sa;
    sa.sa_handler=handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,&sa,NULL);  // cant actually understand how's the signal working.
    if(!network_init(&res->sockfd,(&addr))){
        pool_destroy(pool);
        exit(EXIT_FAILURE);
    }
    socklen_t socklen=sizeof(addr);
    
    while(!stop){
        resp_use_t *client_res;
        client_res=(resp_use_t*)malloc(sizeof(resp_use_t));
        client_res->sockfd=res->sockfd;
        res->new_socket=accept(res->sockfd,(struct sockaddr*)&addr,&socklen);
        client_res->new_socket=res->new_socket;
        if(client_res->new_socket>=0){
            if(client_res->new_socket==0){
                char *res_msg=
                    "HTTP/1.0 503 Service Unvailable\r\n"
                    "Content-Type: text/plain\r\n"
                    "\r\n"
                    "Service Busy";
                send(client_res->new_socket,res_msg,strlen(res_msg),MSG_NOSIGNAL);
                close(client_res->new_socket);
                free(client_res);
            }
            else{
                if(addTask(respond_to_client,client_res)){
                    //printf("\nNEXT %d",count++);
                }
                else{
                    char *res_msg=
                        "HTTP/1.0 503 Service Unvailable\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "Service Busy";
                    send(client_res->new_socket,res_msg,strlen(res_msg),MSG_NOSIGNAL);
                    close(client_res->new_socket);
                    free(client_res);
                }
            }
        }
        else{
            if(errno==EINTR){
                continue;
            }
            perror("accept :");
            break;
        }

    }
    pool->shutdown=1;
    pool_destroy(pool);
    printf("\npool kill");
    return 0;
}



