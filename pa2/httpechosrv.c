/* 
 * tcpechosrv.c - A concurrent TCP echo server using threads
 */

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

#define MAXLINE 8192 /* max text line length */
#define MAXBUF 8192  /* max I/O buffer size */
#define LISTENQ 1024 /* second argument to listen() */

int open_listenfd(int port);
void echo(int connfd);
void *thread(void *vargp);

int main(int argc, char **argv)
{
    int listenfd, *connfdp, port, clientlen = sizeof(struct sockaddr_in);
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
        printf("starting loop\n");
        connfdp = malloc(sizeof(int));
        printf("malloc created\n");
        *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        printf("creating thread\n");
        pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* thread routine */
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    echo(connfd);
    close(connfd);
    return NULL;
}

void error(int connfd)
{
    char buf[MAXLINE];
    char emsg[]= "HTTP/1.1 500 Internal Server Error\r\nContent-Type:text/plain\r\nContent-Length:0\r\n\r\n";
    strcpy(buf, emsg);
    write(connfd, buf, strlen(emsg));
}
/*
    get_content_type:
        - finds the content type of the request and puts
        - puts the corresponding type in the buffer
*/
int get_content_typee(char * buf, char * ext)
{
    if(!strcmp(ext, "txt"))
    {
        strcpy(buf, "text/plain");
    }
    else if(!strcmp(ext, "html"))
    {
        strcpy(buf, "text/html");
    }
    else if(!strcmp(ext, "gif"))
    {
        strcpy(buf, "image/gif");
    }
    else if(!strcmp(ext, "png"))
    {
        strcpy(buf, "image/png");
    }
    else if(!strcmp(ext, "jpg"))
    {
        strcpy(buf, "image/jpg");
    }
    else if(!strcmp(ext, "css"))
    {
        strcpy(buf, "text/css");
    }
    else if(!strcmp(ext, "js"))
    {
        strcpy(buf, "appolication/javascript");
    }
    else
    {
        return -1;
    }
    return 0;
    
}


void get_request(int connfd, char *fname, char * version)
{
    FILE *f;
    ssize_t fsize;
    char content_type[MAXBUF];
    char path[MAXBUF];
    char file_extension [MAXBUF];

    if(!strcmp(fname, "/"))
    {
        //grab the default home page
        printf("Displaying Default Homepage");
        f= fopen("www/index.html", "rb");
        // "rb" option is for for reading binary files, is used for all non-text files 
        fseek(f, 0L, SEEK_END);
        //calculating file size
        fsize= ftell(f);
        rewind(f);
        printf("Requested file is %d\n", (int)fsize);
        strcpy(file_extension, "html");
    }

    else
    {
        //find the file specified
        strcat(path, "www");
        printf("path is: %s", path);
        printf("fmane is: %s", fname);
        strcat(path,fname);
        f= fopen(path, "rb");
        if(f==NULL)
        {
            //file could not be open
            printf("file %s could not be opened in get_reqest fuction", fname);
            error(connfd);
        }
        //calculate the size of file and toss it in header
        fseek(f, 0L, SEEK_END);
        fsize= ftell(f);
        rewind(f);
        printf("Filesize: %d\n", (int)fsize);
        
        //extension starts after the '.' character, so find refrence to that point
        char *front = strchr(fname, '.');
        if(!front)
        {
            printf("file %s could not opened in get_request fuction: BAD EXTENSION\n");
            error(connfd);
            return;
        }
        strncpy(file_extension, front+1, 4);
        }
        printf("File extension: %s\n", file_extension);

        if(get_content_typee(content_type, file_extension)==-1)
        {
            printf("this extension %s is not supported\n", file_extension);
            error(connfd);
            return;
        }
        printf("content type: %s\n", content_type);
        char  contents[fsize];
        fread(contents, 1, fsize, f);
        char header[MAXBUF];
        sprintf(header, "%s 200 Document Follows\r\nContent-Type:%s\r\nContent-Length:%ld\r\n\r\n", version, content_type, fsize);
        char response[fsize +strlen(header)];
        strcpy(response, header); //this fails with non-text files so we add the next line to take care of that.
        memcpy(response +strlen(header), contents, fsize);
        printf("sending server response\n");
        write(connfd, response, fsize+strlen(header));
}




/*
 * echo - read and echo text lines until client closes connection
 */
void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    printf(buf);
    n = read(connfd, buf, MAXLINE);
    printf("server got request: %s\n", buf);
    char *request = strtok(buf, " ");//request type
    char *fname= strtok(buf, " "); //filename
    char *version= strtok(buf, "\r");
    printf("Filename: %s, Version: %s, Request type: %s\n", fname, version, request);


    if(fname ==NULL || version==NULL)
    {
        printf("NULL filename of version\n");
        error(connfd);
        return;
    }
    if(!strcmp(version, "HTTP/1.1") || !strcmp(version, "HTTP/1.0"))
    {
        // do the thing
        if(!strcmp(request, "GET"))
        {
            get_request(connfd,fname, version);
        }
    }
    else
    {
        printf("Invalid request in echo function\n");
        error(connfd);

    }
    
}

/* 
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure 
 */
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
} /* end open_listenfd */
