#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void domainNameToIp(char* domain);
void parsing(int fd,int server_fd);
void readAndWriteRequest(rio_t *rp,int server_fd);

int main(int argc,char **argv) {
    int listenfd, connfd, server_fd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        server_fd = Open_clientfd(hostname, 80);
        parsing(connfd,server_fd);
        Close(connfd);  // line:netp:tiny:close
    }
}

void parsing(int fd,int server_fd){
    int is_static;//현재 들어온 HTTP 요청이 정적인지 동적인지 판단
    struct stat sbuf;//HTTP 요청으로 들어온 file에 대한 정보를 저장하는 구조체
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio,server_rio;
    Rio_readinitb(&rio, fd);
    Rio_readinitb(&server_rio, server_fd);
    readAndWriteRequest(&rio,server_fd);
}

void readAndWriteRequest(rio_t *rp,int server_fd){
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s\n", buf);
    while (strcmp(buf, "\r\n") != 0) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
        Rio_writen(server_fd, buf, strlen(buf));
    }
    return;
}
void domainNameToIp(char* domain){
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc,flags;

    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((rc = getaddrinfo(domain, NULL, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(rc));
        exit(1);
    }

    flags = NI_NUMERICHOST;
    for (p = listp; p; p = p->ai_next) {
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    }
    Freeaddrinfo(listp);
    exit(0);
}

