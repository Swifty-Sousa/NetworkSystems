//Author: Christian F. Sousa
//Network Systems PA3
// much of this code is based off of the exaple code in previous assignments
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* for fgets */
#include <strings.h>	/* for bzero, bcopy */
#include <unistd.h>		/* for read, write */
#include <sys/socket.h> /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define MAXLINE 8192 /* max text line length */
#define MAXBUF 8192	 /* max I/O buffer size */
#define LISTENQ 1024 /* second argument to listen() */

//function delcalarations
int open_listenfd(int port);

void proxy(int connfd);

void *thread(void *vargp);

void send_error(int connfd, char *msg);

int server_connect(char *hostname, int connfd);

bool blacklisted(char *hostname, char *ipaddr);

//function
int main(int argc, char **argv) // the main, duhh
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
		connfdp = static_cast<int *>(malloc(sizeof(int)));
		*connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		pthread_create(&tid, NULL, thread, connfdp);
	}
}

bool blacklisted(char *hostname, char *ipaddr)
{
	FILE *file;
	char line[100];
	char *ret_char;
	if (access("blacklist", F_OK) == -1)
	{
		printf("No blacklist found\n");
		return 0;
	}
	file = fopen("blacklist", "r");
	while (fgets(line, sizeof(line), file))
	{
		ret_char = strchr(line, '\n');
		if (ret_char != NULL)
		{
			*ret_char = '\0';
		}
		if (!strcmp(line, hostname) || !strcmp(line, ipaddr))
		{
			printf("Blacklist match found: %s\n", line);
			return 1;
		}
	}
	fclose(file);
	printf("No blacklist match found\n");
	return 0;
}

void send_error(int connfd, char *msg)
{
	char errormsg[MAXLINE];
	sprintf(errormsg, "HTTP/1.1 %s\r\nContent-Type:text/plain\r\nContent-Length:0\r\n\r\n", msg);
	write(connfd, errormsg, strlen(errormsg));
}

/* thread routine */
void *thread(void *vargp)
{
	int connfd = *((int *)vargp);
	pthread_detach(pthread_self());
	proxy(connfd);
	close(connfd);
	return NULL;
}

void proxy(int connfd)
{

	//~~~~~~~~~~~~~~~parsing request~~~~~~~~~~~~~~~~~~
	size_t n;
	char buf[MAXBUF];
	char *is_slash;
	char *hostname;
	char *ipaddr;
	char uri[MAXBUF];
	bzero(uri, MAXBUF);
	char *holder;
	strcpy(uri, "/");
	n = read(connfd, buf, MAXBUF);
	printf("proxy got request: \n\n%s\n", buf);
	//comes in as GET http://hostname/uri HTTP/1.0
	char *request = strtok(buf, " ");  //Must be "GET"
	char *url = strtok(NULL, " ") + 7; //removes http:// and collects the url
	//char *holder= strtok(NULL," "); //collects uri and adds / removed above
	//char *uri= strstr(slash, holder);
	//char *uri= sprintf(uri,"%s%S", "/",holder);
	char *version = strtok(NULL, "\r"); //collects version.

	is_slash = strchr(url, '/');
	if (is_slash != NULL)
	{
		hostname = strtok(url, "/");
		holder = strtok(NULL, " ");
		strcat(uri, holder);
	}
	else
	{
		hostname = url;
	}
	printf("\n\n\n~~~~~~~~~~~REQUEST INFORMATION~~~~~~~~~~~~~~~~~~\n\n");
	printf("request: %s\n", request);
	//printf("url: %s\n", url);
	printf("hostname: %s\n", hostname);
	printf("uri: %s\n", uri);
	printf("version: %s\n", version);
	struct hostent *gbhn = gethostbyname(hostname);
	ipaddr = inet_ntoa(*(struct in_addr *)gbhn->h_name);
	printf("Ip adress: %s\n\n\n", ipaddr);
	//https://stackoverflow.com/questions/10403238/unix-how-to-get-ip-address-of-domain-name is how I got to the above

	if (strcmp(request, "GET"))
	{
		printf("Unsupported request type\n");
		send_error(connfd, "server only supports GET request");
		return;
	}
	/*
	if (strcmp(version, "HTTP/1.0")) {
		printf("Invalid version of HTTP\n");
		send_error(connfd, "Unsupported HTTP version");
		return;
	}
*/
	if (strlen(hostname) == 0)
	{
		printf("No host requested\n");
		send_error(connfd, "No host requested");
		return;
	}
	if (blacklisted(hostname, ipaddr))
	{
		send_error(connfd, "ERROR 403 FORBIDDEN");
		return;
	}

	//~~~~~~~~~~~~~~~~~~~~~~Connecting to server~~~~~~~~~~~~~~~~~~~~

	printf("we made it to connecting to the server\n");
	int servfd = server_connect(hostname, connfd);

	//~~~~~~~~~~~~~~~~~~~~~sending request to server~~~~~~~~~~~~~~~~
	char buff[MAXBUF]; //outgoing message buffer
	bzero(buff, MAXBUF);
	strcpy(buff, request);							 //"GET"
	strcat(buff, " ");								 //"GET "
	strcat(buff, uri);								 // "GET /graphics/html.gif"
	strcat(buff, " ");								 // "GET /graphics.html.gif "
	strcat(buff, version);							 //"GET /graphics.html.gif"
	strcat(buff, "\r\n\r\n");						 // add end delimitor
	printf("sending request to server: %s\n", buff); //prints out as expected
	// request buffer is made, now to send the message to the server.
	int flag = write(servfd, buff, strlen(buff));
	if (flag < 0)
	{
		printf("error in sending request from proxy to server\n");
		send_error(connfd, "error in sending request from proxy to server");
		return;
	}
	else
	{
		printf("file sent to server sucessfully\n");
	}
	printf("attempting to receive from server\n");
	char server_buffer[MAXBUF];
	while ((flag = read(servfd, server_buffer, MAXBUF)) > 0)
	{
		//printf("text\n");

		if (flag < 0)
		{
			printf("could not recive from server\n");
			send_error(connfd, "could not recive from server");
		}
	}
	//printf("did we make it to here?\n");
	printf("\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~Server Reply~~~~~~~~~~~~~~~~~~\n\n\n");
	printf("data recived fom server: %s\n\n", server_buffer);
	//printf("Data recived from server\n");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~forwarding to client~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


	flag= write(connfd,server_buffer,strlen(server_buffer));
	if(flag<0)
	{
		perror("Error");
		printf("error forwarding traffic to client");
	}
	printf("traffic sent to client");
	close(servfd);
	return;
}

int server_connect(char *hostname, int connfd)
{
	//most of this code taken from the given code in pa1
	int servfd;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	/* socket: create the socket */
	servfd = socket(AF_INET, SOCK_STREAM, 0);
	if (servfd < 0)
	{
		send_error(connfd, "ERROR opening socket");
	}

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL)
	{
		fprintf(stderr, "ERROR, no such host as %s\n", hostname);
		exit(0);
	}

	/* build the server's Internet address */
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(80);
	int server_size = sizeof(serveraddr);
	int flag = connect(servfd, (struct sockaddr *)&serveraddr, server_size);
	printf("connect result is: %d\n", flag);
	if (flag < 0)
	{
		printf("could not connect to host");
		send_error(connfd, "could not connect to host");
	}
	return servfd;
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