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
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

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
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
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
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented", "this method is not implement");
        return;
    }
    read_requesthdrs(&rio);

    /*
     * Parse URI form GET 요청
     * */
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) { // 들어온 파일이 로컬 디스크 상에 없을 경우 에러내고 리턴
        clienterror(fd, filename, "404", "NOT FOUND", "Server could not find this file");
        return;
    }
    /*
     * 정적 컨텐츠 요청
     * */
    if (is_static) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {//file에 대한 접근 권한이 없는 경우
            clienterror(fd, filename, "403", "Forbidden", "Sever could not read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    /*
     * 동적 컨텐츠 요청
     * */
    else{
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Sever could not run the file");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}
