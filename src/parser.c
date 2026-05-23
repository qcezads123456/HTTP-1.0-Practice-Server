#define _GNU_SOURCE
#include "parser.h"
typedef enum{
    GET,
    UNKNOWN,
}method;
typedef enum{
    JS,
    HTML,
    CSS,
}path_t;
typedef struct {
    char method_str[20];
    char path_str[256];
    char version_str[20];
    method s_m;
    path_t s_p;
}request_line_t;

typedef struct{
    char req_str[4096];
    request_line_t *req;
}HTTP_t;

void method_state_init(HTTP_t *msg){
    if(!strcmp(msg->req->method_str,"GET")){
        msg->req->s_m=GET;
    }
    else{
        msg->req->s_m=UNKNOWN;
    }
}
int parse(int new_socket,HTTP_t *msg){
    size_t byte_size=read(new_socket,msg->req_str,sizeof(msg->req_str));
    msg->req_str[byte_size]='\0';
    if(byte_size>0){
        int ret=sscanf(msg->req_str,"%19s %255s %19s",msg->req->method_str,msg->req->path_str,msg->req->version_str);
        if(ret==3){
            method_state_init(msg);
        }
        else{
            perror("string split error");
            return -1;
        }
        if(msg->req->s_m==UNKNOWN){
            return 0; //not get response 404
        }
        else{
            return 1; // respond 200
        }
    }
    else{
        return -1; // read error response 404
    }
}
int path_distinguish(HTTP_t *msg){
    if(strstr(msg->req->path_str,"js")!=NULL){
        msg->req->s_p=JS;
        return 1; // success
    }
    if(strstr(msg->req->path_str,"css")!=NULL){
        msg->req->s_p=CSS;
        return 1; // success
    }
    return -1; // fail
}
int Whitelist_check(HTTP_t *msg){
    char list_path[]="./.www/list.txt";
    struct stat f_st;
    FILE *Flist=fopen(list_path,"r");
    stat(list_path, &f_st);
    char list_content[f_st.st_size+1];
    memset(list_content,0,f_st.st_size+1);
    fread(list_content,sizeof(char),f_st.st_size,Flist);
    char *result=strstr(list_content,msg->req->path_str);
    int return_index;
    if(result==NULL){
        return_index=-1;
    }
    else{
        return_index=1;
    }
    fclose(Flist);
    return return_index;
}
void respond_to_client(void *arg){
    resp_use_t *res=(resp_use_t*)arg;
    HTTP_t *msg;
    msg=(HTTP_t*)malloc(sizeof(HTTP_t));
    msg->req=(request_line_t*)malloc(sizeof(request_line_t));
    char general_dir[]="./.www";
    struct stat st;
    memset(msg->req_str,0,sizeof(msg->req_str));
    if(parse(res->new_socket,msg)<1){
        char error_meg[]="HTTP/1.0 404 Not Found\r\n\r\n";
        send(res->new_socket,error_meg,strlen(error_meg),MSG_NOSIGNAL);
        free(msg->req);
        free(msg);
        shutdown(res->new_socket, SHUT_RDWR);
        close(res->new_socket);
        free(res);
        return;
    }
    else{
        if(!strcmp(msg->req->path_str,"/")){
            msg->req->s_p=HTML;
            char html_dir[]="./.www/index.html";
            FILE *res_file=fopen(html_dir,"r");
            stat(html_dir, &st);
            char content[st.st_size+1];
            memset(content,0,st.st_size+1);
            fread(content,sizeof(char),st.st_size,res_file);
            char res_header[4096];
            snprintf(res_header,sizeof(res_header),
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n\r\n"
            ,st.st_size);
            ssize_t n;
            send(res->new_socket,res_header,strlen(res_header),MSG_NOSIGNAL);
            send(res->new_socket,content,strlen(content),MSG_NOSIGNAL);
            fclose(res_file);
            free(msg->req);
            free(msg);
            shutdown(res->new_socket, SHUT_RDWR);
            close(res->new_socket);
            free(res);
            return;
        }
        else{
            int path_index=path_distinguish(msg);
            int whitelist_index=Whitelist_check(msg);
            if(path_index<1 || whitelist_index<1){
                char error_meg[]="HTTP/1.0 404 Not Found\r\n\r\n";
                ssize_t n;
                n=send(res->new_socket,error_meg,strlen(error_meg),MSG_NOSIGNAL);
                free(msg->req);
                free(msg);
                shutdown(res->new_socket, SHUT_RDWR);
                close(res->new_socket);
                free(res);
                return;
            }
            else{
                char specific_path[512];
                memset(specific_path,0,256);
                snprintf(specific_path,sizeof(specific_path),"%s%s",general_dir,msg->req->path_str);
                FILE *res_file=fopen(specific_path,"r");
                stat(specific_path, &st);
                char content[st.st_size+1];
                char res_header[4096];
                memset(content,0,st.st_size+1);
                fread(content,sizeof(char),st.st_size,res_file);
                switch (msg->req->s_p)
                {
                case JS:
                    snprintf(res_header,sizeof(res_header),
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: application/javascript\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n\r\n"
                    ,st.st_size);
                    break;
                
                case CSS:
                    snprintf(res_header,sizeof(res_header),
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: text/css\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n\r\n"
                    ,st.st_size);
                    break;

                default:
                    break;
                }
                send(res->new_socket,res_header,strlen(res_header),MSG_NOSIGNAL);
                send(res->new_socket,content,strlen(content),MSG_NOSIGNAL);
                fclose(res_file);
                free(msg->req);
                free(msg);
                shutdown(res->new_socket, SHUT_RDWR);
                close(res->new_socket);
                free(res);
                return;
            }
        }
    }

}

//use fopen load list.txt and fread to list[] ,
// then use strstr() to compare path