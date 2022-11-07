#include "csapp.h"

int main(void)
{
    char *buf, *p;
    char *method;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;
    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL)
    {
        p = strchr(buf, '&');
        *p = '\0';
        strcpy(arg1, buf);
        strcpy(arg2, p+1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }
    method = getenv("REQUEST_METHOD");
    /* Make the response body */
    sprintf(content, "QUERY_STRING=%s", buf);
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal. \r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>",
            content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting! \r\n", content);

    if (strcasecmp(method, "HEAD") == 0) {
        sprintf(content, "%sConnection: close\r\n", content);
        sprintf(content, "%sContent-length: %lu\r\n", content, strlen(content));
        sprintf(content, "%sContent-type: text/html\r\n\r\n", content);
        printf("Connection : close\r\n");
        printf("Content-length: %d\r\n", (int)strlen(content));
        printf("Content-type : text/html \r\n\r\n");

        fflush(stdout);

        exit(0);
    }
    /* Generate the HTTP response */
    printf("Connection : close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type : text/html \r\n\r\n");
    printf("%s", content);
    fflush(stdout);
    exit(0);
}