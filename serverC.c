#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <math.h>
#include "csvparser.h"
#define MAXLINE 1024 
#define PORT    23319  
#define AWS_PORT    20319  

int main() { 
	int sockfd;  //Server socket file descriptor
	char buffer[MAXLINE]; 
	struct sockaddr_in servaddr, cliaddr, aws_servaddr; 
	int n, len, ret;

	char linkid[MAXLINE], size[MAXLINE], spower[MAXLINE];
	float filesize, band, velocity, length, noise_power, power;
	float prop_delay, trans_delay, end_delay;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { /* UDP socket creation */
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&servaddr, 0, sizeof(aws_servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 

	/* Filling current server information */
	servaddr.sin_family    = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(PORT); 

	/* Filling aws server information */
	aws_servaddr.sin_family    = AF_INET; // IPv4 
	aws_servaddr.sin_addr.s_addr = INADDR_ANY; 
	aws_servaddr.sin_port = htons(AWS_PORT); 

	/* Bind the socket with the server address */
	if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	printf("The Server C is up and running using UDP on port <%d>\n", PORT);

	while(1) {
		/* Receving message from AWS */
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
		buffer[n] = '\0';
		sscanf(buffer, "%s %s %s", linkid, size, spower);
		sscanf(buffer, "%*s %f %f %*s %f %f %f %f", &filesize, &power, &band, &length, &velocity, &noise_power);
		printf("The Server C received link information of link <%s>, file size <%s>, and signal power <%s>\n", linkid, size, spower);
		
		/* Calculating propogation delay */
		prop_delay = (length/velocity) * 1000;
		
		/* Converting powers from dBm to watt */
		power = (pow(10, (power/10)))/1000;
		noise_power = (pow(10, (noise_power/10)))/1000;

		/* Calculating transmission delay */
		trans_delay = (filesize/(band*(log2(1 + (power/noise_power))))) * 1000;

		/* Calculating end to end delay */
		end_delay = prop_delay + trans_delay;

		printf("The Server C finished the calculation for link <%s>\n", linkid);

		snprintf(buffer, sizeof(buffer), "%f %f %f", prop_delay, trans_delay, end_delay);
		/* Sending results to AWS server */
		sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *) &aws_servaddr, sizeof(aws_servaddr));
		printf("The Server C finished sending the output to AWS\n"); 
	}
	close(sockfd);
	return 0; 
} 
