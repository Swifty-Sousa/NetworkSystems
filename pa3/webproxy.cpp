//Author: Christian F. Sousa
//Network Systems PA3
// much of this code is based off of the exaple code in previous assignments
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* for fgets */
#include <strings.h>    /* for bzero, bcopy */
#include <unistd.h>     /* for read, write */
#include <sys/socket.h> /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <fstream>

#define MAXLINE 8192 /* max text line length */
#define MAXBUF 8192  /* max I/O buffer size */
#define LISTENQ 1024 /* second argument to listen() */


//function delcalarations
int open_listenfd(int port);
void test(int connfd);
void *thread(void *vargp);

//function
int main(int argc, char** argv)// the main, duhh
{
    int listenfd, *connfdp, port;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);
    //jump to bottom and create a listning port
    listenfd = open_listenfd(port);
    
    printf("listening port open\n");
    
    while (1)
    {
        connfdp = static_cast<int*>(malloc(sizeof(int)));
        *connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* thread routine */
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());
    //free(vargp);
    //echo(connfd);
    test(connfd);
    close(connfd);
    return NULL;
}

void test(int connfd)
{
    size_t n;
    char buf [MAXLINE];
    n=read(connfd, buf, MAXLINE);
    printf("proxy got request: \n\n%s\n", buf);
}



//connects proxy to client
int open_listenfd(int port)
{
    printf("beginning open\n");
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    printf("terminating open\n");
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} 