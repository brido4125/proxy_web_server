/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static_get(int fd, char *filename, int filesize);
void serve_static_head(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs,char * method);
void clientError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, short headFlag);

int main(int argc, char **argv) {
  int listenfd, connfd;
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
      printf("connection fd : %d\n", (int) connfd);
      Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
      printf("Accepted connection from (%s, %s)\n", hostname, port);
      /*
       * / 요청이 아닌 경우, 임의의 connection이 생긴다.
       * */
      doit(connfd);
      Close(connfd);  // line:netp:tiny:close
  }
}

/*
 * 한개의 HTTP 트랜잭션을 처리
 * */
void doit(int fd){
    int is_static;//현재 들어온 HTTP 요청이 정적인지 동적인지 판단
    struct stat sbuf;//HTTP 요청으로 들어온 file에 대한 정보를 저장하는 구조체
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
    /*
     * HTTP 요청을 읽음(HTTP 헤더 파싱)
     * */
    Rio_readinitb(&rio, fd);
    /* buffer 비우기 */
    if (rio.rio_buf[0] != '\0') {
        rio.rio_buf[0] = '\0';
    }
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    short getFlag = strcasecmp(method, "GET");
    short headFlag = strcasecmp(method, "HEAD");
    if (getFlag != 0 && headFlag != 0) {
        clientError(fd, method, "501", "Not Implemented", "this method is not implement",headFlag);
        return;
    }
    read_requesthdrs(&rio);

    /*
     * Parse URI form GET 요청
     * */
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) { // 들어온 파일이 로컬 디스크 상에 없을 경우 에러내고 리턴
        if (headFlag == 0) {
            clientError(fd, filename, "404", "NOT FOUND", "Server could not find this file",headFlag);
            return;
        }
        clientError(fd, filename, "404", "NOT FOUND", "Server could not find this file",headFlag);
        return;
    }
    /*
     * 정적 컨텐츠 요청
     * */
    if (is_static) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {//file에 대한 접근 권한이 없는 경우
            if (headFlag == 0) {
                clientError(fd, filename, "403", "Forbidden", "Sever could not read the file",headFlag);
                return;
            }
            clientError(fd, filename, "403", "Forbidden", "Sever could not read the file",headFlag);
            return;
        }
        if (headFlag == 0) {
            serve_static_head(fd, filename, sbuf.st_size);
        } else{
            serve_static_get(fd, filename, sbuf.st_size);
        }

    }
    /*
     * 동적 컨텐츠 요청
     * */
    else{
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            if (headFlag == 0) {
                clientError(fd, filename, "403", "Forbidden", "Sever could not run the file",headFlag);
                return;
            }
            clientError(fd, filename, "403", "Forbidden", "Sever could not run the file",headFlag);
            return;
        }
        serve_dynamic(fd, filename, cgiargs,method);
    }
}

void clientError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, short headFlag){
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body*/
    sprintf(body,"<html><title>Tiny Error</title>");
    sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
    sprintf(body,"%s%s: %s\r\n",body,errnum,shortmsg);
    sprintf(body,"%s<p>%s: %s\r\n",body,longmsg,cause);
    sprintf(body,"%s<hr><em>The Tiny Web Server</em>\r\n",body);

    /* Print the HTTP response*/
    sprintf(buf,"HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf,"Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf,"Content-length: %d\r\n\r\n",(int) strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    if (headFlag == 0) {
        return;
    }
    Rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp){
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

int parse_uri(char *uri, char *filename, char *cgiargs){
    char* ptr;
    /*
     * 현재 uri가 "cgi-bin"을 포함하지 않으면 => 정적 컨텐츠 요청
     * */
    if (!strstr(uri, "cgi-bin")) {
        strcpy(cgiargs, "");
        strcpy(filename,".");
        strcat(filename,uri);
        if (uri[strlen(uri) - 1] == '/') {
            strcat(filename, "home.html");
            printf("parseFileName : %s\n", filename);
        }
        return 1;
    }
    /*동적 컨텐츠 요청*/
    else{
        ptr = index(uri, '?');//query parameter가 시작하는 지점 찾기
        if (ptr) {
            strcpy(cgiargs, ptr + 1);//cgiargs부터 query-params가 들어오는 포인터 = ptr + 1
            *ptr = '\0';
        }
        else{
            strcpy(cgiargs, "");
        }
        /*
         * Filename을 상대주소로 변환
         * */
        strcpy(filename,".");
        strcat(filename,uri);
        return 0;
    }
}

void serve_static_head(int fd, char *filename, int filesize){
    char filetype[MAXLINE], buf[MAXBUF];

    /* Send Response 헤더 */
    get_filetype(filename, filetype);
    sprintf(buf,"HTTP/1.0 200 OK\r\n");//시작줄 설정
    sprintf(buf,"%sServer: Tiny Web Server\r\n",buf);
    sprintf(buf,"%sConnection: close\r\n",buf);
    sprintf(buf,"%sContent-length: %d\r\n",buf,filesize);
    /*
     * \r\n\r\n 하는 이유 : HTTP 헤더의 집합은 항상 빈 줄로 끝나야함
     * */
    sprintf(buf,"%sContent-type: %s\r\n\r\n",buf,filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);
}



void serve_static_get(int fd, char *filename, int filesize){
    int srcfd;
    char *srcp;

    serve_static_head(fd, filename, filesize);

    /* Send response body to client*/
    srcfd = Open(filename, O_RDONLY, 0);
    //srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    srcp = (char*)malloc(filesize);
    Rio_readn(srcfd, srcp, filesize);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    //Munmap(srcp, filesize);// 매핑된 가상메모리 주소를 반환한다.
    free(srcp);
}

void get_filetype(char *filename, char *filetype){
    /* File Type : HTML 파일*/
    if (strstr(filename, ".html")) {
        strcpy(filetype, "text/html");
    }
    /* File Type : GIF 파일*/
    else if (strstr(filename, ".gif")) {
        strcpy(filetype, "image/gif");
    }
    /* File Type : PNG 파일*/
    else if (strstr(filename, ".png")) {
        strcpy(filetype, "image/png");
    }
    /* File Type : JPG 파일*/
    else if (strstr(filename, ".jpg")) {
        strcpy(filetype, "image/jpeg");
    }
    /* 11.7 동영상 처리*/
    else if (strstr(filename, ".mp4")) {
        strcpy(filetype, "video/mp4");
    }
        /* File Type : 무형식 텍스트 파일*/
    else {
        strcpy(filetype, "text/plain");
    }
}


void serve_dynamic(int fd, char *filename, char *cgiargs,char * method){
    char buf[MAXLINE], *emptylist[] = {NULL};

    /* Return first part of HTTP response*/
    sprintf(buf,"HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf,"Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Child에게 CGI 프로그램 시킴*/
    if (Fork() == 0) {
        setenv("QUERY_STRING", cgiargs, 1);
        setenv("REQUEST_METHOD", method, 1);
        Dup2(fd, STDOUT_FILENO);
        Execve(filename, emptylist, environ);
    }
    Wait(NULL);
}