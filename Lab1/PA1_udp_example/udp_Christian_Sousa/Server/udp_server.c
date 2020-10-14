/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <dirent.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) 
{
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char cmd[BUFSIZE];// command from client
  char filename[BUFSIZE]; //filename for command from client
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) 
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
  {
    error("ERROR opening socket");
  }

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) 
  {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);




	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SERVER GET~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	sscanf(buf, "%s %s",cmd, filename);
	//printf("we get here\n");
	//printf(buf);
	if(!strncmp(cmd, "get", 3))
	{
		    FILE* file = fopen(filename, "rb");
            if(file==NULL)
            {
                error("File does not exist in directory");
            }
            clientlen = sizeof(clientaddr);
            //n= -1 if sendto or revice error out
    		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, clientlen);
    		if (n < 0) 
			{
  	    		error("ERROR in  PUT sendto");
			}
            fseek(file, 0L, SEEK_END);
            int f_size= ftell(file);
            rewind(file);
            char fbuf [BUFSIZE];
            sprintf(fbuf, "%d", f_size);
    		n = sendto(sockfd, fbuf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, clientlen);
            bzero(fbuf,sizeof(fbuf));
            while(fread(fbuf,1,sizeof(fbuf),file)!=0)
            {
    		    n = sendto(sockfd, fbuf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, clientlen);
                bzero(fbuf,sizeof(fbuf));
            }
            fclose(file);
            printf("file transfered succesfully\n");
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SERVER PUT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if(!strncmp(cmd, "put", 3))
	{
		/* send the message to the server */
    		clientlen = sizeof(clientaddr);
            //n= -1 if sendto or revice error out
    		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, clientlen);;
    		if (n < 0) 
			{
  	    		error("ERROR in sendto");
			}
            /* print the server's reply */
    	     n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&clientaddr, &clientlen);
            if (n < 0) 
		    {
      	        error("ERROR in recvfrom");
		    }
            //server will send back an error if the file is not found
            //server will response with file size if it is found
            if(!strncmp(buf, "DNE",3))            
            {
                printf("The file name input does not exist on remote server\n");
                continue;
            }
            int size= atoi(buf);
            FILE * file= fopen(filename, "wb");
            int btrans=0;// counter for how many bytes have been transfered

            while(true)
            {
                int tsize= BUFSIZE; //the size of the transfer
                if(size-btrans< BUFSIZE)
                {
                    tsize= size - btrans;// determine how large the transfer will be
                }
    	        n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&clientaddr, &clientlen);
                fwrite(buf,tsize,1,file);
                btrans+=tsize;
                if(btrans>=size)
                {
                    printf("file transfered\n");
                    fclose(file);
                    break;
                }
			}
	}




	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SERVER DELETE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if(!strncmp(cmd,"delete",6))
	{
		bzero(buf,sizeof(buf));
		FILE* file= fopen(filename, "rb");
		if(file==NULL)
		{
			error("ERROR File does not exisit");
			strncpy(buf, "DNE", 3);
    		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, clientlen);
			continue;
		}
		remove(filename);
    bzero(buf,sizeof(buf));
    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, clientlen);

	}




	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SERVER LS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if(!strncmp(cmd, "ls", 2))
	{
		bzero(buf, sizeof(buf));
		DIR* direct= opendir(".");
		while(true)
		{
			struct dirent* dir = readdir(direct);
			if(dir==NULL)
			{
				break;
			}
			strcat(buf,dir->d_name);
			strncat(buf,"\n",1);
		}
    	n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, clientlen);

	}




	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SERVER EXIT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


	else if(!strncmp(cmd,"exit", 4))
	{
		printf("exit char recived, ending process\n");
		break;
	}
  }
  return 0;
}
