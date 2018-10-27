#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 26319 
#define MAXLINE 1024 

int main(int argc, char const *argv[]) { 
	int sockfd = 0; /* Socket descriptor */
	int valread; 
	struct sockaddr_in serv_addr; 
	char buffer[1024] = {0};
	char nomatch[1024];
	char linkid[MAXLINE], size[MAXLINE], power[MAXLINE];
	char prop_delay[MAXLINE], trans_delay[MAXLINE], end_delay[MAXLINE];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* TCP Socket creation */ 
		printf("\n TCP Socket creation error \n"); 
		return -1; 
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET; //IP4
	serv_addr.sin_port = htons(PORT); 

	/* Convert IPv4 address from text to binary form */
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) { 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { /* Connecting to server */
		printf("\nConnection Failed \n"); 
		return -1; 
	}

	printf("The Monitor is up and running\n");
	while(1) {
		valread = read(sockfd, buffer, 1024);
		sscanf(buffer, "%s %s %s", linkid, size, power);
		printf("The monitor received input=<%s>, size=<%s>, and power=<%s> from the AWS\n", linkid, size, power);
		valread = read(sockfd, nomatch, 1024);
		if (strcmp(nomatch,"No match") == 0)
			printf("Found no matches for link <%s>\n", linkid);
		else {
			sscanf(nomatch, "%s %s %s", prop_delay, trans_delay, end_delay);
			printf("The result for link <%s>:\nTt = <%s>ms\nTt = <%s>ms\nDelay = <%s>ms\n", linkid, trans_delay, prop_delay, end_delay);
		}
	}
	close(sockfd);
	return 0; 
} 
