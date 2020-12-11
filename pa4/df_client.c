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
int sockfd[4];
struct sockaddr_in serveraddr[4];
char usrname[BUF];
char passwd[BUF];




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTION DECLARATIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void socket_init(int id, char* ip, int port);
int hash_func(char * file);
int auth(char * uname, char * passwd, int id);
int does_block_exist(char *f_name, int block, int id);
int request_block(FILE *inf, int block_size, int id);
void send_blk(char * blk, char *filename, char *blk_id, size_t blk_len, int id);


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
            //printf("Port is: %d\n",ports[D1]);
            holder= strtok(line, " ");
            holder= strtok(NULL, " ");
            holder= strtok(NULL, " ");
            holder2= strtok(holder, ":");
            strcpy(holder3,holder2);
            ips[D1]= holder3;
            //printf("IP is: %s\n", ips[D1]);
        }
        if(i ==D2)
        {
            //printf("line 2: %s\n", line);
            char holder4[16]="";
            ports[D2]= atoi(strchr(line, ':') +1);
            //printf("Port is: %d\n",ports[D2]);
            holder= strtok(line, " ");
            holder= strtok(NULL, " ");
            holder= strtok(NULL, " ");
            holder2= strtok(holder, ":");
            strcpy(holder4,holder2);
            ips[D2]= holder4;
            //printf("IP is: %s\n", ips[D2]);
        }
        if(i==D3)
        {
            char holder5[16]="";
            ports[D3]= atoi(strchr(line, ':') +1);
            //printf("Port is: %d\n",ports[D3]);
            holder= strtok(line, " ");
            holder= strtok(NULL, " ");
            holder= strtok(NULL, " ");
            holder2= strtok(holder, ":");
            strcpy(holder5,holder2);
            ips[D3]= holder5;
            //printf("IP is: %s\n", ips[D3]);
        }
        if(i == D4)
        {
            char holder6[16]="";
            ports[D4]= atoi(strchr(line, ':') +1);
            //printf("Port is: %d\n",ports[D4]);
            holder= strtok(line, " ");
            holder= strtok(NULL, " ");
            holder= strtok(NULL, " ");
            holder2= strtok(holder, ":");
            strcpy(holder6,holder2);
            ips[D4]= holder6;
            //printf("IP is: %s\n", ips[D4]);

        }
        if (i==(D4+1))
        {
            //this is for the username
            strncpy(usrname,strchr(line, ' ')+1, BUF);
            printf("Username: %s\n",usrname);
        }
        if(i==(D4+2))
        {
            strncpy(passwd,strchr(line, ' ')+1, BUF);
            printf("password: %s\n", passwd);
        }
        
    }
    fclose(config);
    int test=-1;
    test= hash_func("eve.conf");
    printf("Did hash_func, hash is: %d\n", test);
    test= hash_func("test.txt");
    printf("Did hash_func, hash is: %d\n", test);
    socket_init(D1, ips[D1], ports[D1]);
    socket_init(D2, ips[D2], ports[D2]);
    socket_init(D3, ips[D3], ports[D3]);
    socket_init(D4, ips[D4], ports[D4]);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~End setup loop~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //init for driver logic vars:
    char ui[BUF]; //user input
    char command[BUF]; // the command the user gives after first contact
    char file[BUF];//filename

    while(true)
    {
        bzero(ui, BUF);
        bzero(command, BUF);
        printf("Please select one of the following commands:\n\n");
        printf("get <filename>\n");
        printf("put <filename>\n");
        printf("ls\n");
        fgets(ui, BUF, stdin);
        sscanf(ui, "%s %s", command, file);
        //printf("command is %s  \nfile is %s\n", command, file);
        printf("command is %s\n", command);
        if(auth(usrname,passwd,D1)==0)
        {
            printf("Authentication failed\n");
            continue;
        }
        printf("past first auth\n");
        if(auth(usrname,passwd,D2)==0)
        {
        printf("Authentication failed\n");
            continue;
        }
        if(auth(usrname,passwd,D3)==0)
        {
        printf("Authentication failed\n");
            continue;
        }
        if(auth(usrname,passwd,D4)==0)
        {
            printf("Authentication failed\n");
            continue;
        }
        printf("authorized");
        if(!strcmp(command,"get"))
        {

            FILE *inf= fopen(file,"wb");
            if(inf==NULL)
            {
                printf("file: %s failed to open",file);
            }   
            bool flag;
            //for each block in order
                //check each server for the chunk
                    //if it exists slap that boi in the file and go next
            for(int i=1; i<=4; i++)
            {
                flag=false;
                for(int j=0; i<=3;j++)        
                {
                    int c_size= does_block_exist(file,i,j);
                    if(c_size>0)
                    {
                        flag=true;
                        request_block(inf,c_size,j);
                        break;
                    }
                }
                if(flag== false)
                {
                    printf("file: %s does not exist or cannot be rebuilt\n",file);
                }
            }
            fclose(inf);
        }



        if(!strcmp(command,"put"))
        { 
            //printf("do we make it into put\n");
            FILE *of;
            of= fopen(file, "rb");
            //printf("we try to open file yes\n");
            printf("we open file\n");
            struct stat s;
            if(of==NULL)
            {
                printf("file %s could not be opened for put\n");
                continue;
            }
            //printf("is the issue with seek\n");
            fseek(of,0, SEEK_END);
            //printf("one \n");
            ssize_t file_size= ftell(of);
            //printf("two \n");
            rewind(of);
            //printf("we mess with blocks\n");
            //tells me how big the file is

            int block_size= file_size/3;
            char blk_1[block_size];
            char blk_2[block_size];
            char blk_3[block_size];
            char blk_4[(file_size-(3*block_size))];
            //printf("broken up into blocks\n");
            fread(blk_1,block_size,1,of);
            fread(blk_2,block_size,1,of);
            fread(blk_3,block_size,1,of);
            fread(blk_4,sizeof(blk_4),1,of);
            fclose(of);
            //printf("reading done\n");
            //splits up file into chunks
            int hash= hash_func(file);
            if(hash==0)
            {
                send_blk(blk_1,file,"1",sizeof(blk_1), D1);
                send_blk(blk_2,file,"2",sizeof(blk_2), D1);

                send_blk(blk_2,file,"2",sizeof(blk_2), D2);
                send_blk(blk_3,file,"3",sizeof(blk_3), D2);

                send_blk(blk_3,file,"3",sizeof(blk_3), D3);
                send_blk(blk_4,file,"4",sizeof(blk_4), D3);

                send_blk(blk_4,file,"4",sizeof(blk_4), D4);
                send_blk(blk_1,file,"1",sizeof(blk_1), D4);
                //finished
            }
            if(hash==1)
            {
                send_blk(blk_4,file,"4",sizeof(blk_4), D1);
                send_blk(blk_1,file,"1",sizeof(blk_1), D1);

                send_blk(blk_1,file,"1",sizeof(blk_1), D2);
                send_blk(blk_2,file,"2",sizeof(blk_2), D2);

                send_blk(blk_2,file,"2",sizeof(blk_2), D3);
                send_blk(blk_3,file,"3",sizeof(blk_3), D3);

                send_blk(blk_3,file,"3",sizeof(blk_3), D4);
                send_blk(blk_4,file,"4",sizeof(blk_4), D4);
                //finshed

            }
            if(hash==2)
            {
                send_blk(blk_3,file,"3",sizeof(blk_3), D1);
                send_blk(blk_4,file,"4",sizeof(blk_4), D1);

                send_blk(blk_4,file,"4",sizeof(blk_4), D2);
                send_blk(blk_1,file,"1",sizeof(blk_1), D2);

                send_blk(blk_1,file,"1",sizeof(blk_1), D3);
                send_blk(blk_2,file,"2",sizeof(blk_2), D3);

                send_blk(blk_2,file,"2",sizeof(blk_2), D4);
                send_blk(blk_3,file,"3",sizeof(blk_3), D4);
                //finished

            }
            if(hash==3)
            {
                send_blk(blk_2,file,"2",sizeof(blk_2), D1);
                send_blk(blk_3,file,"3",sizeof(blk_3), D1);

                send_blk(blk_3,file,"3",sizeof(blk_3), D2);
                send_blk(blk_4,file,"4",sizeof(blk_4), D2);

                send_blk(blk_4,file,"4",sizeof(blk_4), D3);
                send_blk(blk_1,file,"1",sizeof(blk_1), D3);

                send_blk(blk_1,file,"1",sizeof(blk_1), D4);
                send_blk(blk_2,file,"2",sizeof(blk_2), D4);
                //finished

            }
            
            printf("blocks sent\n");
        }
        
        
        if(!strcmp(command, "ls"))
        {
            char f1[MAXLINE];
            char f2[MAXLINE];
            char f3[MAXLINE];
            char f4[MAXLINE];
            bzero(f1,MAXLINE);
            bzero(f2,MAXLINE);
            bzero(f3,MAXLINE);
            bzero(f4,MAXLINE);

            write(D1,"ls",2);
            read(sockfd[D1],f1,MAXLINE);
            write(D2,"ls",2);
            read(sockfd[D2],f1,MAXLINE);
            write(D3,"ls",2);
            read(sockfd[D3],f1,MAXLINE);
            write(D4,"ls",2);
            read(sockfd[D4],f1,MAXLINE);
            //writes down all files on each server;
            char *line;
            int file_count=0;
            char files[32][32]; //stores files 
            memset(files, 0, sizeof(files));
            int file_blk_counter[32];
            bzero(file_blk_counter, 32);
            if(strlen(f1) != 0)
            {
                printf("f1 is: %s", f1);
                line=strtok(f1,"|");
                while(line!=NULL)
                {
                    bool new_flag=true;
                    char *blk= strrchr(line, ',')+1;
                    *(blk-1)= '\0';
                    for(int i=0; i<file_count; i++)
                    {
                        if(!strcmp(line, files[i]))
                        {
                            file_blk_counter[i]++;
                            new_flag=false;
                            break;
                        }
                    }
                    if(new_flag)
                    {
                       strcpy(files[file_count], line);
                       file_blk_counter[file_count]=1;
                       file_count++;
                    }
                    line= strtok(NULL, "|");

                }


            }
            if(strlen(f2) != 0)
            {
                printf("f1 is: %s", f1);
                line=strtok(f1,"|");
                while(line!=NULL)
                {
                    bool new_flag=true;
                    char *blk= strrchr(line, ',')+1;
                    *(blk-1)= '\0';
                    for(int i=0; i<file_count; i++)
                    {
                        if(!strcmp(line, files[i]))
                        {
                            file_blk_counter[i]++;
                            new_flag=false;
                            break;
                        }
                    }
                    if(new_flag)
                    {
                       strcpy(files[file_count], line);
                       file_blk_counter[file_count]=1;
                       file_count++;
                    }
                    line= strtok(NULL, "|");

                }


            }

            if(strlen(f3) != 0)
            {
                printf("f1 is: %s", f1);
                line=strtok(f1,"|");
                while(line!=NULL)
                {
                    bool new_flag=true;
                    char *blk= strrchr(line, ',')+1;
                    *(blk-1)= '\0';
                    for(int i=0; i<file_count; i++)
                    {
                        if(!strcmp(line, files[i]))
                        {
                            file_blk_counter[i]++;
                            new_flag=false;
                            break;
                        }
                    }
                    if(new_flag)
                    {
                       strcpy(files[file_count], line);
                       file_blk_counter[file_count]=1;
                       file_count++;
                    }
                    line= strtok(NULL, "|");

                }


            }
            if(strlen(f4) != 0)
            {
                printf("f1 is: %s", f1);
                line=strtok(f1,"|");
                while(line!=NULL)
                {
                    bool new_flag=true;
                    char *blk= strrchr(line, ',')+1;
                    *(blk-1)= '\0';
                    for(int i=0; i<file_count; i++)
                    {
                        if(!strcmp(line, files[i]))
                        {
                            file_blk_counter[i]++;
                            new_flag=false;
                            break;
                        }
                    }
                    if(new_flag)
                    {
                       strcpy(files[file_count], line);
                       file_blk_counter[file_count]=1;
                       file_count++;
                    }
                    line= strtok(NULL, "|");

                }


            }
            
            for(int i=0; i< file_count; i++)
            {
                printf("%s", file[i]);
                if(file_blk_counter[i]!=4)
                {
                    printf("[Incomplete file]\n");
                }
                printf("\n");
            }


        }
        
        
        else
        {
            printf("Invalid command please try again.\n");
            continue;
        }
    }

}

void send_blk(char * blk, char* filename, char *blk_id, size_t blk_len, int id)
{
    printf("we in send_blk\n");
    char size[BUF];
    sprintf(size,"%d", (int)blk_len);
    int cmd_size= strlen("put ") + blk_len +5;// for .blk_id
    char cmd[cmd_size];
    //put filename|data.blk_id
    strcpy(cmd, "put ");
    printf("is this the issue\n");
    strcat(cmd, size);
    printf("maybe not\n");
    strcat(cmd, " ");
    strcat(cmd,filename);
    strcat(cmd,".");
    strcat(cmd,blk_id);
    strcat(cmd,"|");
    strcat(cmd,blk);
    printf("sending block: %s",cmd);
    write(sockfd[id],cmd,sizeof(cmd));
}


int request_block(FILE *inf, int block_size, int id)
{
    char block[block_size];
    bzero(block, block_size);
    write(sockfd[id],"send traffic", 12);
    read(sockfd[id],block,block_size);
    fwrite(block,1,block_size,inf);
}

int does_block_exist(char *f_name, int block_id, int id)
{
    int msg_len= strlen("get ") + strlen("1.") + strlen(f_name);
    char block_buf[msg_len];
    bzero(block_buf,sizeof(block_buf));
    char ack[BUF];
    bzero(ack,BUF);
    char cblock_id= (char) block_id;
    write(sockfd[id],block_buf,sizeof(block_buf));
    int size=read(sockfd[id],ack,BUF);
    if(size<=0)
    {
        printf("DFS%d is not responding or file is not present\n");
        return -1;
    }
    else if(!strncmp(ack, "blk_size|", 9))
    {
        char *dat_pointer= strchr(ack, '|');
        if(dat_pointer==NULL)
        {
            return 0;
        }
        int holder = atoi(dat_pointer +1);
        //advance the pointer by 1 to avoid the "|"
        return holder;
    }
    else
    {
        return 0;
    }
}



int auth(char *uname, char* pass, int id)
{
    //printf("do we make it to auth\n");
    // "|" will be the separator char in this function
    int msg_len= strlen("auth|")+ strlen(uname) +strlen(passwd) +2; 
    // added 2 for the | between user and pass as well as for carriage return
    char authenticate[msg_len];
    char ack[BUF];// servers reply
    bzero(ack, BUF);
    strcpy(authenticate, "auth ");
    strcat(authenticate,uname);
    strcat(authenticate, "|");
    strcat(authenticate,passwd);
    //printf("authenticate is: %s", authenticate);
    write(sockfd[id],authenticate,sizeof(authenticate));
    ssize_t size= read(sockfd[id], ack,BUF);
    if(size<=0)
    {
        printf("error in client authenticator\n");
    }
    if(!strcmp(ack, "authorized"))
    {
        return 1;
    }
    else
    {
        printf("could not authenticate this user\n");
        return -1;
    }
    
}


void socket_init(int id, char* ip, int port)
{
    sockfd[id]= socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server= gethostbyname(ip);
    //got socket timeout from here: https://stackoverflow.com/questions/4181784/how-to-set-socket-timeout-in-c-when-making-multiple-connections
    struct timeval timeout;
    timeout.tv_sec=10;
    timeout.tv_usec= 0;
    
    if(setsockopt(sockfd[id], SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout))<0)
    {
        printf("socket timeout\n");
        return;
    }
    
    bzero((char *)&serveraddr[id], sizeof(struct sockaddr_in));
    serveraddr[id].sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr[id].sin_addr.s_addr, server->h_length);
    serveraddr[id].sin_port = htons(port);
    int flag =connect(sockfd[id], (struct sockaddr *)&serveraddr[id], sizeof(serveraddr));
    if(flag<0)
    {
        printf("connect failed\n");
    }
    else
    {
        printf("connected\n");
    }
    
}

// this bit of code came from here: https://stackoverflow.com/questions/10324611/how-to-calculate-the-md5-hash-of-a-large-file-in-c

int hash_func(char * file)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    FILE* f= fopen(file, "rb");
    MD5_CTX context;
    unsigned char data[1024];
    int bytes;
    if(f==NULL)
    {
        printf("file %s could not be opened\n", f);
    }
    MD5_Init(&context);
    while((bytes==fread(data,1,1020,f))!=0)
    {
        MD5_Update(&context, data, bytes);
    }
    MD5_Final(c,&context);
    //https://www.geeksforgeeks.org/generating-random-number-range-c/
    //random number generation;
    int result=0;
    srand(time(NULL));
    int r_num;
    for(int i=0; i<MD5_DIGEST_LENGTH; i++)
    {
        int holder= abs((int)c[i]);
        r_num=(rand()%(10 - 1+ 1))+1;
        result=(result *r_num + holder);
    }
    result= abs(result);
    result= (result)%4;
    return result;
}


