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
char path[4]; // /ds1
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTION DECLARATIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int open_listenfd(int port);
void * thread(void* vargp);
void * make_usr_dir(char *usrname);
bool auth(char * buf, char * uname_buf);
void df_server(int connfd);
void serve(int connfd);
void check_dir(char * uname);

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

    portno = atoi(argv[2]);
    listenfd = open_listenfd(portno);
    strcpy(path, argv[1]);

    while (1)
    {
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfdp);
    }
}


void *thread(void *vargp)
{
	int connfd = *((int *)vargp);
	pthread_detach(pthread_self());
    serve(connfd);
	close(connfd);
	return NULL;
}

void serve(int connfd)
{
    char usr_name[BUF];
    while(true)
    {
        char buf[MAXLINE];
        bzero(buf, MAXLINE);
        size_t flag= read(connfd, buf, MAXLINE);
        printf("message recived: %s\n", buf);
        if(flag==0)
        {
            break;
        }
        //printf("command buffer is: %s");

        if (!strncmp(buf, "auth", 4))
        {
            if(auth(buf,usr_name))
            {
                write(connfd, "authorized", 10);
            }
            else
            {
                write(connfd, "failed", 6);
            }
            
        }
        if(!strncmp(buf,"get", 3))
        {
            printf("we made it to get\n");
            struct stat test;
            int file_size;
            char * filename= strchr(buf, ' ')+1 ;
            int holder= strlen(filename) +strlen(path) +strlen(usr_name)+4;
            char path_to_f[holder];
            strcpy(path_to_f, ".");
            strcat(path_to_f, path);
            strcat(path_to_f, "/");
            strcat(path_to_f, usr_name);
            strcat(path_to_f, "/");
            strcat(path_to_f, filename);
            if(stat(path_to_f, &test)==0)
            {
                file_size= (int)test.st_size;
                if(file_size==0)
                {
                    printf("file does not exist on server\n");
                    write(connfd,"blk_size|", 9);
                    continue;
                }
                char size_buf[32];
                write(connfd, size_buf, sizeof(size_buf));
            }
            char blk[file_size];
            bzero(blk, sizeof(blk));
            FILE * blk_file =fopen(path_to_f, "rb");
            fread(blk, file_size, 1, blk_file);
            char traffic[12];
            read(connfd,traffic, 12);
            write(connfd, blk_file, sizeof(blk_file));

        }



        if(!strncmp(buf,"put",3))
        {
            check_dir(usr_name);
            char * file_name= strchr(buf, ' ')+1;
            *(file_name -1 )= '\0';
            char *fsize= strchr(buf, ' ')+1;
            int file_size= atoi(fsize);
            int holder= strlen(path) + strlen(usr_name) +strlen(file_name) +5;
            char dir[holder];
            bzero(dir, sizeof(dir));
            strcpy(dir, ".");
            strcat(dir, path);
            strcat(dir, "/");
            strcat(dir, usr_name);
            strcat(dir, "/");
            strcat(dir, file_name);
            FILE *file= fopen(dir, "wb");
            char f_buf[file_size];
            int size= read(connfd, f_buf, file_size);
            fwrite(f_buf,1,size, file);
            fclose(file);

        }
        if(!strncmp(buf, "ls",2))
        {
            check_dir(usr_name);
            char buff[MAXLINE];
            bzero(buff, MAXLINE);
            int holder= strlen(path) +strlen(usr_name) +3;
            char f_path[holder];
            strcpy(f_path, ".");
            strcat(f_path, path);
            strcat(f_path, "/");
            strcat(f_path, usr_name);
            strcat(f_path, "/");
            DIR *d=opendir(f_path);
            while(true)
            {
                struct  dirent *dir =readdir(d);
                if(dir==NULL)
                {
                    break;
                }
                if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, ".."))
                {
                    strcat(buff, dir->d_name);
                    strcat(buf, "|");
                }
                
            }

        }
    }
}

void check_dir(char * uname)
{
    DIR *dir;
    int holder= strlen(path)+strlen(uname)+ 3;
    char d_path[holder];
    bzero(d_path, holder);
    strcpy(d_path,path+1);
    strcat(d_path, "/");
    strcat(d_path, uname);
    dir= opendir(d_path);
    if(dir)
    {
        closedir(dir);
    }
    else
    {
        mkdir(d_path, 0777);
    }
    
}

bool auth(char *buf, char *uname_buff)
{
    printf("are we getting to auth\n");
    char * line=NULL;
    char *uname= strchr(buf, ' ')+1;
    char *psswd= strchr(buf, '|')+1;
    *(psswd -1)= '\0';
    strncpy(uname_buff, uname, BUF);
    FILE *config;
    ssize_t bytes;
    ssize_t len=0;
    config= fopen("dfs.conf", "r");
    while((bytes =getline(&line, &len, config))!= -1)
    {
        int holder= strcspn(line, "\n");
        line[holder]=0;
        char *c_psswd= strchr(line, ' ')+1;
        *(c_psswd -1) ='\0';
        if(!strcmp(uname, line) &&!strcmp(psswd,c_psswd))
        {
            printf("authentication successful\n");
            return true;
        }
    }

    return false;
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