#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 8192
#define BUF 256
#define LISTENQ 1024

// above includes taken from previous provided code

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTION DECLARATIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int open_listenfd(int port);
void * thread(void* vargp);
void * make_usr_dir(char *usrname);
bool auth();
void df_server(int connfd);

//main also taken from previous given code.
int main(int argc, char **argv)
{
    int listenfd, *connfdp, portno, clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;

    if (argc != 3)
    {
        printf("Invalid number of arguments - usage: ./dfs </DFS{1-4}> <port #>\n;");
        return 1;
    }

    if (strcmp(argv[1], "1") && strcmp(argv[1], "2") && strcmp(argv[1], "3") && strcmp(argv[1], "4"))
    {
        printf("Usage: ./dfs <1-4> <port #>\n");
        return 1;
    }
    portno = atoi(argv[2]);
    listenfd = open_listenfd(portno);

    while (1)
    {
        printf("Attempting to connect.\n");
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfdp);
    }
}


void *thread(void *vargp)
{
	int connfd = *((int *)vargp);
	pthread_detach(pthread_self());
	df_server(connfd);
	close(connfd);
	return NULL;
}

void df_server(int connfd)
{
    while(true)
    {
        size_t n;
        char buf[MAXLINE];
    }
    auth();
    //stuff goes here
}


void *make_usr_dir(char * username)
{
    //stuff goes here
}

bool auth()
{
    printf("we make it to auth\n");
    //stuff goes here
    return true;
}



















// Yoinked from the PA 1 given code.


/*
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure
 */
int open_listenfd(int port)
{
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port on any IP address for this host */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */
