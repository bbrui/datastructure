#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 10001

#define MAX_BUF 4096

/**
 * 
 * @param request, the string to send.
 * @param response, the response received.
 * 
 * @return the length of response 
 */
 

int send_request(char *request, char *response)
{
	fd_set readfds;
	struct timeval timeout;
	int n =  0, nfds = 0;
	int len; //receive length
	
	int sockfd = -1, numbytes, totalbytes, done;
	struct sockaddr_in my_addr;
	struct sockaddr_in dest_addr;
	int destport = SERVER_PORT;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return -1;
	}
	
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(destport);
	dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	
	memset(&dest_addr.sin_zero, 0, 8);
	
	if ((connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr))) == -1)
	{
		perror("connect");
		return -5;
	}

	send(sockfd, request, strlen(request), 0);
	

	//memset(request, 0, sizeof(request));
	
	numbytes = totalbytes = 0;
	done = 0;
	do {
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		timeout.tv_sec = 30; /* XXX magic... 30 second is as good a timeout as any */
		timeout.tv_usec = 0;
		nfds = sockfd + 1;

		nfds = select(nfds, &readfds, NULL, NULL, &timeout);

		if (nfds > 0) {
			/** We don't have to use FD_ISSET() because there
			 *  was only one fd. */
			numbytes = read(sockfd, response + totalbytes, MAX_BUF - (totalbytes + 1));
			if (numbytes < 0) {
				printf("An error occurred while reading from auth server: %s", strerror(errno));
				/* FIXME */
				close(sockfd);
				return (-1);
			}
			else if (numbytes == 0) {
				done = 1;
			}
			else {
				totalbytes += numbytes;
				printf("Read %d bytes, total now %d", numbytes, totalbytes);
			}
		}
		else if (nfds == 0) {
			printf("Timed out reading data via select() from auth server");
			/* FIXME */
			close(sockfd);
			return (-2);
		}
		else if (nfds < 0) {
			printf("Error reading data via select() from auth server: %s", strerror(errno));
			/* FIXME */
			close(sockfd);
			return (-3);
		}
	} while (!done);
	close(sockfd);
	
	printf("Done reading reply, total %d bytes", totalbytes);

	if (totalbytes >= MAX_BUF)
		totalbytes = MAX_BUF;
	response[totalbytes] = '\0';
	printf("HTTP Response from Server: [%s]", response);
	
	return totalbytes;
}


int upgrade()
{
	char request[MAX_BUF];
	char content[MAX_BUF-1024];
	int len = 0;
	memset(request, 0, sizeof(request));
	memset(content, 0, sizeof(content));
	
	snprintf(content, sizeof(content), 
	"{\"version\":\"xxx\",\"mac\":\"aa:cc:bb:dd:ee:ff\",\"hw\":\"7620a\"}"
	);
	
	/* Form http request header */
	snprintf(request, sizeof(request)-1, "POST /dqdata/upgrade HTTP/1.1\r\n"
								 "Host: %s\r\n"
								 "ContentLength: %d\r\n"
								 "ContentType: text/json\r\n\r\n"
								 "%s", 
								 SERVER_IP, strlen(content), content);
								 
	len = send_request(request, request);
	
	printf("Response[%d]=%s\n", len, request);
	request[MAX_BUF] = '\0';
	
	return 0;
}

int main(int argc, char *argv[])
{

	upgrade();

	
	
	return 0;
}
