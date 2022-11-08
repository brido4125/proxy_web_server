#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void domainNameToIp(char* domain);
void doit(int fd);
void readAndWriteRequest(rio_t *rp,int server_fd);
void read_requesthdrs(rio_t *rp);
//void make_request_to_sever(rio_t *rp);
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
    char *token = strtok(uri, ":");//포트번호 없이 uri만 가지는 문자열
    printf("token = %s", token);
    strcpy(uri,token);
    printf("port : %s \n", port);
    printf("%s\n", uri);
    printf("======Request From Client=======\n");
    printf("%s", buf);
    read_requesthdrs(&clientRio);
    printf("======Request To Server=======\n");


    //server_fd = Open_clientfd(hostname, port);


    //make_request_to_sever(&serverRio);
    //Rio_readinitb(&serverRio, server_fd);
    //readAndWriteRequest(&clientRio,server_fd);
}


/*void make_request_to_sever(rio_t *rp){
    char filetype[MAXLINE], buf[MAXBUF];
*//* Send Response 헤더 *//*
    sprintf(buf,"HTTP/1.0 200 OK\r\n");//시작줄 설정
    sprintf(buf,"%sServer: Tiny Web Server\r\n",buf);
    sprintf(buf,"%sConnection: close\r\n",buf);
    *//*
     * \r\n\r\n 하는 이유 : HTTP 헤더의 집합은 항상 빈 줄로 끝나야함
     * *//*
    sprintf(buf,"%sContent-type: %s\r\n\r\n",buf,filetype);
    //Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    Rio_writen(server_fd, buf, strlen(buf));
    while(strcmp(buf, "\r\n"))
    {
        Rio_readlineb(&rp, buf, MAXLINE);
        ssacnf(content, "%s%s", content, buf);
        printf("%s", buf)

    }
}*/

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

