#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
static const char* proxy_host;
static const char* proxy_port;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void domainNameToIp(char* domain);
void doit(int fd);
void readAndWriteRequest(rio_t *rp,int server_fd);
void read_requesthdrs(rio_t *rp);
void make_request_to_sever(rio_t *rp,char* host,int serverFd);
char* get_port_number(char* s, int start, int end);

int main(int argc,char **argv) {
    int listenfd, connfd;//FD 값 선언
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char origin_hostname[MAXLINE], origin_port[MAXLINE];

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    proxy_port = argv[0];

    listenfd = Open_listenfd(argv[1]);//getAddressInfo가 내부 호출됨

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, origin_hostname, MAXLINE, origin_port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", origin_hostname, origin_port);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int fd){
    int serverFd;
    struct stat sbuf;//HTTP 요청으로 들어온 file에 대한 정보를 저장하는 구조체
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE],userAgent[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    char hostname[MAXLINE], port[MAXLINE];
    rio_t clientRio,serverRio;

    Rio_readinitb(&clientRio, fd);
    Rio_readlineb(&clientRio, buf, MAXLINE);
    sscanf(buf, "%s http://%s %s", method, uri, version);
    if (strcasecmp(method, "GET") != 0) {
        printf("%s does not implemented\n", method);
        return;
    }
    printf("%s\n", uri);
    char *portIndex = index(uri, ':');
    if (portIndex == NULL) {
        strcpy(port,"80");
    }else{
        strncpy(port, portIndex + 1, 5);
    }
    char *ipAddress = strtok(uri, ":");//포트번호 없이 uri만 가지는 문자열

    printf("port : %s \n", port);
    printf("%s\n", uri);
    printf("======Request From Client=======\n");
    printf("%s", buf);
    read_requesthdrs(&clientRio);
    printf("======Request To Server=======\n");
    serverFd = Open_clientfd(ipAddress, port);
    printf("serverFd : %d \n", serverFd);
    Rio_readinitb(&serverRio, serverFd);
    make_request_to_sever(&serverRio,uri,serverFd);
}


void make_request_to_sever(rio_t *rp,char* host,int serverFd){
    printf("make_request_to_sever - start \n");
    char buf[MAXBUF];
    sprintf(buf,"GET / HTTP/1.0\r\n");//시작줄 설정
    sprintf(buf,"%sHost: %s\r\n",buf,host);
    sprintf(buf,"%sConnection: close\r\n",buf);
    sprintf(buf,"%sProxy-Connection: close\r\n",buf);
    Rio_writen(serverFd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

}

void read_requesthdrs(rio_t *rp){
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}


