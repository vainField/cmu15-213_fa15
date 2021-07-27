#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";

/************
 * SOLUTION *
 ************/

/* Function Prototype */

void doit(int fd);
void parse_uri(char *uri,char *hostname,char *path,int *port);
void build_requesthdrs(rio_t *rp, char *newreq, char *hostname, char* port);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/* main */
int main(int argc, char** argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    printf("%s", user_agent_hdr);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);

    /* setup proxy server */
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        /* setup proxy server end */
        doit(connfd);
        Close(connfd);
    }
    return 0;
}

/***********
 * HELPERS *
 ***********/

/* handle client static request */
void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], 
        hostname[MAXLINE], path[MAXLINE], port[10], request[MAX_OBJECT_SIZE];
    rio_t rio, rio_client;
    int clientfd;
    int port_num;

    /* read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented", "");
        return;
    }


    /* parse uri from GET request */
    parse_uri(uri, hostname, path, &port_num);
    sprintf(port, "%d", port_num);

    /* proxy client */
    if ((clientfd = Open_clientfd(hostname, port)) < 0) {
        printf("Connection Failed\n");
        return;
    }
    Rio_readinitb(&rio_client, clientfd);

    /* buid request headers */
    sprintf(request, "GET %s HTTP/1.0\r\n", path);
    build_requesthdrs(&rio, request, hostname, port);

    /* send request to Server */
    Rio_writen(clientfd, request, strlen(request));

    /* Server to Client */
    int i;
    while((i = Rio_readlineb(&rio_client, buf, MAXLINE)))
        Rio_writen(fd, buf, i);

    // Close(clientfd);
}

/* parse static request uri */
void parse_uri(char *uri, char *hostname, char *path, int *port) {
    *port = 80;
    char* pos1 = strstr(uri, "//");
    if (pos1 == NULL) {
        pos1 = uri;
    } else pos1 += 2;

    char* pos2 = strstr(pos1, ":");
    if (pos2 != NULL) {
        *pos2 = '\0'; 
        strncpy(hostname, pos1, MAXLINE);
        sscanf(pos2+1, "%d%s", port, path); 
        *pos2 = ':';
    } else {
        pos2 = strstr(pos1,"/");
        if (pos2 == NULL) {
            strncpy(hostname,pos1,MAXLINE);
            strcpy(path,"");
            return;
        }
        *pos2 = '\0';
        strncpy(hostname,pos1,MAXLINE);
        *pos2 = '/';
        strncpy(path,pos2,MAXLINE);
    }
}

/* build request headers */
void build_requesthdrs(rio_t *rp, char *newreq, char *hostname, char* port) {
    //already have sprintf(newreq, "GET %s HTTP/1.0\r\n", path);
    char buf[MAXLINE];

    while (Rio_readlineb(rp, buf, MAXLINE) > 0) {          
        if (!strcmp(buf, "\r\n")) break;
        if (strstr(buf,"Host:") != NULL) continue;
        if (strstr(buf,"User-Agent:") != NULL) continue;
        if (strstr(buf,"Connection:") != NULL) continue;
        if (strstr(buf,"Proxy-Connection:") != NULL) continue;

        sprintf(newreq, "%s%s", newreq, buf);
    }
    sprintf(newreq, "%sHost: %s:%s\r\n", newreq, hostname, port);
    sprintf(newreq, "%s%s%s%s", newreq, user_agent_hdr, conn_hdr, prox_hdr);
    sprintf(newreq, "%s\r\n", newreq);
}

/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}