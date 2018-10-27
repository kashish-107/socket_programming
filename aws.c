#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define UDP_PORT    20319  
#define CLIENT_PORT 25319 
#define MONITOR_PORT 26319 
#define SERVER_A_PORT    21319  
#define SERVER_B_PORT    22319  
#define SERVER_C_PORT    23319  
#define MAXLINE 1024 

void socket_details(struct sockaddr_in *temp, int port) {
	temp->sin_family = AF_INET; // IPv4
	temp->sin_addr.s_addr = INADDR_ANY;
	temp->sin_port = htons(port);
}

int main() { 
	struct sockaddr_in udpservaddr, cliaddr, udpservaddrA, udpservaddrB, udpservaddrC;
	struct sockaddr_in clientservaddr, monitorservaddr; 
	int udpsockfd;  //Udp server socket file descriptor
	int clientsockfd, client_socket, valread;  
	int monitorsockfd, monitor_socket;  
	char buffer[MAXLINE], bufferA[MAXLINE], bufferB[MAXLINE]; 
	char end_delay[MAXLINE]; 
	int opt = 1;
	int len, n, m; 
	int tcp_addrlen = sizeof(clientservaddr);
	char linkid[MAXLINE], size[MAXLINE], power[MAXLINE];
	char *nomatch = "No match";

	if ((udpsockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("UDP socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	if ((clientsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) { 
		perror("Client TCP socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	if ((monitorsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) { 
		perror("Monitor TCP socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	if (setsockopt(clientsockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(monitorsockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	memset(&udpservaddr, 0, sizeof(udpservaddr)); 
	memset(&udpservaddrA, 0, sizeof(udpservaddrA)); 
	memset(&udpservaddrB, 0, sizeof(udpservaddrB)); 
	memset(&udpservaddrC, 0, sizeof(udpservaddrC)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 

	socket_details(&udpservaddr, UDP_PORT);
	socket_details(&udpservaddrA, SERVER_A_PORT);
	socket_details(&udpservaddrB, SERVER_B_PORT);
	socket_details(&udpservaddrC, SERVER_C_PORT);
	socket_details(&clientservaddr, CLIENT_PORT);
	socket_details(&monitorservaddr, MONITOR_PORT);

	if (bind(udpsockfd, (const struct sockaddr *)&udpservaddr, sizeof(udpservaddr)) < 0 ) { 
		perror("UDP socket bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	if (bind(clientsockfd, (struct sockaddr *)&clientservaddr, sizeof(clientservaddr)) < 0) {
		perror("Client socket bind failed");
		exit(EXIT_FAILURE);
	}

	if (bind(monitorsockfd, (struct sockaddr *)&monitorservaddr, sizeof(monitorservaddr)) < 0) {
		perror("Monitor socket bind failed");
		exit(EXIT_FAILURE);
	}

	printf("The AWS is up and running \n");
	if (listen(clientsockfd, 1) < 0) {
		perror("Client listen");
		exit(EXIT_FAILURE);
	}

	if (listen(monitorsockfd, 1) < 0) {
		perror("Monitor listen");
		exit(EXIT_FAILURE);
	}

	if ((monitor_socket = accept(monitorsockfd, (struct sockaddr *)&monitorservaddr, (socklen_t*)&tcp_addrlen)) < 0) {
		perror("Monitor accept");
		exit(EXIT_FAILURE);
	}

	while(1) {
		if ((client_socket = accept(clientsockfd, (struct sockaddr *)&clientservaddr, (socklen_t*)&tcp_addrlen)) < 0) {
			perror("Client accept");
			exit(EXIT_FAILURE);
		}
		/* Reading values from client */
		n = recv(client_socket, buffer, MAXLINE, 0);
		buffer[n] = '\0';
		sscanf(buffer, "%s %s %s", linkid, size, power);
		printf("The AWS received link ID=<%s>, size=<%s>, and power=<%s> from the client using TCP over port <%d>\n", linkid, size, power, CLIENT_PORT);

		/* Sending details to monitor server */
		send(monitor_socket , buffer, strlen(buffer), 0);
		printf("The AWS sent link ID=<%s>, size=<%s>, and power=<%s> to the monitor using TCP over port <%d>\n", linkid, size, power, MONITOR_PORT);

		/* Sending link ID to backend server A */
		sendto(udpsockfd, (const char *)linkid, strlen(linkid), MSG_CONFIRM, (const struct sockaddr *) &udpservaddrA, sizeof(udpservaddrA));
		printf("The AWS sent link ID=<%s> to Backend-Server A using UDP over port < %d > \n", linkid, SERVER_A_PORT);

		/* Receving message from backend server A */
		n = recvfrom(udpsockfd, (char *)bufferA, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len); 
		bufferA[n] = '\0'; 

		/* Sending link ID to backend server B */
		sendto(udpsockfd, (const char *)linkid, strlen(linkid), MSG_CONFIRM, (const struct sockaddr *) &udpservaddrB, sizeof(udpservaddrB));
		printf("The AWS sent link ID=<%s> to Backend-Server B using UDP over port < %d > \n", linkid, SERVER_B_PORT);

		/* Receving message from backend server B */
		m = recvfrom(udpsockfd, (char *)bufferB, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len); 
		bufferB[m] = '\0';

		sscanf(bufferA, "%d", &n);
		sscanf(bufferB, "%d", &m);

		printf("The AWS received < %d > matches from Backend-server < A > using UDP over port < %d >\n", n, SERVER_A_PORT);
		printf("The AWS received < %d > matches from Backend-server < B > using UDP over port < %d >\n", m, SERVER_B_PORT);

		if (n == 1 || m == 1) {
			strcat(buffer, " ");
			if (n == 1) {
				strcat(buffer, bufferA);
			}
			else {
				strcat(buffer, bufferB);
			}
			/* Sending link details to backend server C */
			sendto(udpsockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *) &udpservaddrC, sizeof(udpservaddrC));
			printf("The AWS sent link ID=<%s>, size=<%s>, and power=<%s> and link information to Backend-Server C using UDP over port < %d >\n", linkid, size, power, SERVER_C_PORT);
			/* Receving message from backend server C */
			n = recvfrom(udpsockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len); 
			buffer[n] = '\0'; 
			printf("The AWS received outputs from Backend-Server C using UDP over port < %d >\n", SERVER_C_PORT);

			sscanf(buffer, "%*s %*s %s", end_delay);	

			/* Sending delay to client */
			send(client_socket, end_delay, strlen(end_delay), 0);
			printf("The AWS sent delay=<%s>ms to the client using TCP over port < %d >\n", end_delay, CLIENT_PORT);

			/* Sending details to monitor server */
			send(monitor_socket , buffer, strlen(buffer), 0);
			printf("The AWS sent detailed results to the monitor using TCP over port < %d >\n", MONITOR_PORT); 
		}
		else {
			send(client_socket, nomatch, strlen(nomatch), 0);		
			send(monitor_socket, nomatch, strlen(nomatch), 0);
			printf("The AWS sent No Match to the monitor and the client using TCP over ports <%d> and <%d>, respectively\n", CLIENT_PORT, MONITOR_PORT);
		}
		buffer[0] = '\0';
	}
	close(udpsockfd);
	close(monitorsockfd);
	close(clientsockfd);
	return 0; 
} 
