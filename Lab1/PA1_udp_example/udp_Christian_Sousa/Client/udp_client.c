/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdbool.h>
#include <dirent.h>
#include <arpa/inet.h>


#define BUFSIZE 1024

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}


int main(int argc, char **argv) 
{
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
	char cmd[BUFSIZE]; // string holder for user input command
	char filename[BUFSIZE];// string holder for user input file name
	//char holder[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) 
    {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
      error("ERROR opening socket");
    }

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    while(true)
    {
    	/* get a message from the user */
    	bzero(buf, BUFSIZE);// clears out buffer to prepare for reading
    	printf("Please select an option: \n");
		printf("get [filename]\n");
		printf("put [filename]\n");
		printf("delete [filename]\n");
		printf("ls\n");
		printf("exit\n>");
    	fgets(buf, BUFSIZE, stdin);
		sscanf(buf, "%s %s", cmd, filename);
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GET FUNCTION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if(!strncmp(cmd, "get",3))
		{
    		/* send the message to the server */
    		serverlen = sizeof(serveraddr);
            //n= -1 if sendto or revice error out
    		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);;
    		if (n < 0) 
			{
  	    		error("ERROR in sendto");
			}
            /* print the server's reply */
    	     n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&serveraddr, &serverlen);
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
    	        n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&serveraddr, &serverlen);
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
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~PUT FUNCTION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        else if(!strncmp(cmd, "put", 3))
        {
            FILE* file = fopen(filename, "rb");
            if(file==NULL)
            {
                error("File does not exist in directory");
            }
            serverlen = sizeof(serveraddr);
            //n= -1 if sendto or revice error out
    		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
    		if (n < 0) 
			{
  	    		error("ERROR in  PUT sendto");
			}
            fseek(file, 0L, SEEK_END);
            int f_size= ftell(file);
            rewind(file);
            char fbuf [BUFSIZE];
            sprintf(fbuf, "%d", f_size);
    		n = sendto(sockfd, fbuf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, serverlen);
            bzero(fbuf,sizeof(fbuf));
            while(fread(fbuf,1,sizeof(fbuf),file)!=0)
            {
    		    n = sendto(sockfd, fbuf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, serverlen);
                bzero(fbuf,sizeof(fbuf));
            }
            fclose(file);
            printf("file transfered succesfully\n");

        }

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~DELETE FUNCTION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        else if(!strncmp(cmd,"delete", 6))
        {
            serverlen = sizeof(serveraddr);
            n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
            if (n < 0)
                error("ERROR in DELETE sendto");

            n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&serveraddr, &serverlen);
            if (n < 0)
            {
                error("ERROR in  DELETE recvfrom");
            }
            if(!strncmp(buf,"DNE",3))
            {
                error("ERROR file reqested to delete on server does not exist");
            }
            printf("Deleated File\n");
        }
        


        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LS FUNCTION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        else if(!strncmp(cmd, "ls", 2))    
        {
            serverlen= sizeof(serveraddr);
            n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
            if(n<0)
            {
                error("ERROR in sendto");
            }
            //recive data 
            n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&serveraddr, &serverlen);
            if(n<0)
            {
                error("ERROR in PUT recvfrom");
            }
            printf(buf);
        }


        else if(!strncmp(cmd, "exit", 4))
        {
            serverlen= sizeof(serveraddr);
            n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
            if(n<0)
            {
                error("ERROR in sendto");
             
            }
            printf("Terminating Program\n");
            return 0;
	    }
    }
}
