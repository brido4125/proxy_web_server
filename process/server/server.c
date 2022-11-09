//
// Created by 홍창섭 on 2022/11/03.
//
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "csapp.h"

void echo(int connfd);
void sigchld_handler(int sig);

int main(int argc,char ** argv){
    int listenfd, connfd, port;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);//문자열로 구성된 포트를 정수형으로 변환

    Signal(SIGCHLD, sigchld_handler);
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);//child에게 전달해줄 connection fd
        if (Fork() == 0) {
            Close(listenfd);
            echo(connfd);
            Close(connfd);
            exit(0);
        }
        Close(connfd);
    }
}

void sigchld_handler(int sig){
    while (waitpid(-1, 0, WNOHANG) > 0) {
        ;
    }
    return;
}

void echo(int connfd){
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", (int) n);
        Rio_writen(connfd, buf, n);
    }
}