#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 25319 

int main(int argc, char const *argv[]) { 
	int sockfd = 0; /* Socket descriptor */
	int valread; 
	struct sockaddr_in serv_addr; 
	char buffer[1024] = {0};
	char nomatch[1024] = {0};

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

	printf("The client is up and running\n");
	printf("The client sent link ID=<%s>,size=<%s>, and power=<%s> to AWS\n",argv[1], argv[2], argv[3]);
	snprintf(buffer, sizeof(buffer), "%s %s %s", argv[1], argv[2], argv[3]); 
	valread = send(sockfd, buffer, strlen(buffer), 0); /* Sending client input to AWS */

	valread = read(sockfd, nomatch, 1024); /* Reading message from AWS */

	if (strcmp(nomatch,"No match") == 0) /* Checking whether results found or not */
		printf("Found no matches for link <%s>\n", argv[1]);
	else
		printf("The delay for link <%s> is <%s>ms\n", argv[1], nomatch);

	close(sockfd); /* Closing socket */
	return 0; 
} 
