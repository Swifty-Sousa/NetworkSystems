#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <netdb.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h>

#define MAXLINE 8192
#define BUF 256
#define MAXFILES 128
#define MAXFILENAME 64

//ID's too keep my understanding straight of which D server I am talking about
#define D1 0
#define D2 1
#define D3 2
#define D4 3


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Global Vars~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int sockfd[3];
struct sockaddr_in serveraddr[3];




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTION DECLARATIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int socket_init(char * ip, int port);


int main(int argc, char **argv)
{
    int ports[4];
    char *ips[4];
    
    if(argc!=2)
    {
        printf("Usage: ./df_client <config file>\n");
        return 1;
    }
    FILE *config;
    char *line=NULL;
    ssize_t leng;
    config= fopen(argv[1],"r");
    if(config==NULL)
    {
        printf("Config file could not be opened.\n");
    }
    //we can use i<6 because the files are of a static size
    char* holder;
    char* holder2;
    for(int i=0; i<6; i++)
    {
        getline(&line,&leng,config);
        line[strcspn(line,"\n")]=0; // this removes any trailing \n chars
        if(i == D1)
        {

            char holder3[16]="";
            ports[D1]= atoi(strchr(line, ':') +1);
            printf("Port is: %d\n",ports[D1]);
            holder= strtok(line, " ");
            holder= strtok(NULL, " ");
            holder= strtok(NULL, " ");
            holder2= strtok(holder, ":");
            strcpy(holder3,holder2);
            ips[D1]= holder3;
            printf("IP is: %s\n", ips[D1]);
        }
        if(i ==D2)
        {
            printf("line 2: %s\n", line);
            char holder4[16]="";
            ports[D2]= atoi(strchr(line, ':') +1);
            printf("Port is: %d\n",ports[D2]);
            holder= strtok(line, " ");
            holder= strtok(NULL, " ");
            holder= strtok(NULL, " ");
            holder2= strtok(holder, ":");
            strcpy(holder4,holder2);
            ips[D2]= holder4;
            printf("IP is: %s\n", ips[D2]);
        }
        if(i==D3)
        {
            char holder5[16]="";
            ports[D3]= atoi(strchr(line, ':') +1);
            printf("Port is: %d\n",ports[D3]);
            holder= strtok(line, " ");
            holder= strtok(NULL, " ");
            holder= strtok(NULL, " ");
            holder2= strtok(holder, ":");
            strcpy(holder5,holder2);
            ips[D3]= holder5;
            printf("IP is: %s\n", ips[D3]);
        }
        if(i == D4)
        {
            char holder6[16]="";
            ports[D4]= atoi(strchr(line, ':') +1);
            printf("Port is: %d\n",ports[D4]);
            holder= strtok(line, " ");
            holder= strtok(NULL, " ");
            holder= strtok(NULL, " ");
            holder2= strtok(holder, ":");
            strcpy(holder6,holder2);
            ips[D4]= holder6;
            printf("IP is: %s\n", ips[D4]);

        }
    }







    return 0;
}

int socket_init(char *ip, int port)
{
    int sockfd= socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    struct hostent *server= gethostbyname(ip);
    
    
    bzero((char *)&serveraddr, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(port);
    int flag =connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(flag<0)
    {
        printf("connect failed\n");
    }
    else
    {
        printf("connected\n");
    }
    
    return sockfd;
}