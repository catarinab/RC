#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define MAX_INPUT_SIZE 128

int udpSocket, tcpSocket, errcode, errno;
struct addrinfo hints, *udpRes, *tcpRes;
ssize_t n;
socklen_t addrlen;
struct sockaddr_in addr;
char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_INPUT_SIZE]; //nosso pc: localhost;

int main(int argc, char *argv[]) {

	//parse command line arguments
	if(argc == 1) {
		if(gethostname(ip, 128) == -1) fprintf(stderr, "error: %s\n", strerror(errno));
		strcpy(port, "58056");
	}
	else if (argc == 5) {
		if(strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-p") == 0) {
			strcpy(ip, argv[2]);
			strcpy(port, argv[4]);
		} 
		else if(strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-n") == 0) {
			strcpy(port, argv[2]);
			strcpy(ip, argv[4]);
		} 
		else fprintf(stderr, "error: incorrect command line arguments\n");
	}
	else if (argc == 3) {
		if(strcmp(argv[1], "-n") == 0) strcpy(ip, argv[2]);
		else if(strcmp(argv[1], "-p") == 0) strcpy(port, argv[2]);
		else fprintf(stderr, "error: incorrect command line arguments\n");
	}
	else fprintf(stderr, "error: incorrect command line arguments\n");

	//sockets creation
	//UDP
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(udpSocket == -1) exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	errcode = getaddrinfo(ip, port, &hints, &udpRes);
	if(errcode == -1) exit(1);

	/*
	//TCP
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(tcpSocket == -1) exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	errcode = getaddrinfo(ip, port, &hints, &tcpRes);
	if(errcode == -1) exit(1);
	*/


    while (fgets(buffer, sizeof(buffer)/sizeof(char), stdin)) {
		char op[MAX_INPUT_SIZE], args[2][MAX_INPUT_SIZE], command[MAX_INPUT_SIZE] = "";
        int numTokens = sscanf(buffer, "%s %s %s", op, args[0], args[1]);

        if (numTokens < 1) fprintf(stderr, "error: incorrect command line arguments\n");
		else {
			for(int i = 0; i < numTokens-1; i++){
				strcat(command, " ");
				strcat(command, args[0]);
			}

			char finalCommand[MAX_INPUT_SIZE];
			if(strcmp(op, "reg") == 0) {
				strcpy(finalCommand, "REG");
				strcat(finalCommand, command);
				n = sendto(udpSocket, finalCommand, strlen(finalCommand), 0, udpRes->ai_addr, udpRes->ai_addrlen);
				if(n == -1) exit(1);
				addrlen = sizeof(addr);
				n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
				if(n == -1) exit(1);
				numTokens = sscanf(buffer, "%s %s", op, args[0]);
				if(numTokens != 2 || strcmp(op, "RRG") != 0) exit(1);
				else if(strcmp(args[0], "OK") == 0){
					fprintf(stdout, "User Successfully Registered.\n");
				}
				else if(strcmp(args[0], "DUP") == 0){
					fprintf(stdout, "User Already Registered.\n");
				}
				else if(strcmp(args[0], "NOK") == 0){
					fprintf(stdout, "User Not Registered.\n");
				}
				else exit(1);

			}
		}
	} 

	freeaddrinfo(udpRes);
	freeaddrinfo(tcpRes);
	close(udpSocket);
	close(tcpSocket);

	return 0;
}