#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#define MAX_INPUT_SIZE 128

int udpSocket, tcpSocket, errcode, errno;
struct addrinfo hints, *udpRes, *tcpRes;
ssize_t n;
socklen_t addrlen;
struct sockaddr_in addr;
char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_INPUT_SIZE]; //nosso pc: localhost;

void parseArgs(int argc, char *argv[]) {
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
}

void createSockets() {
	//UDP
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(udpSocket == -1) exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	errcode = getaddrinfo(ip, port, &hints, &udpRes);
	if(errcode == -1) exit(1);

	//TCP
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(tcpSocket == -1) exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	errcode = getaddrinfo(ip, port, &hints, &tcpRes);
	if(errcode == -1) exit(1);
}

void deleteSockets() {
	freeaddrinfo(udpRes);
	freeaddrinfo(tcpRes);
	close(udpSocket);
	close(tcpSocket);
}

void concatenateArgs(char *final, int numArgs, char args[2][MAX_INPUT_SIZE]) {
	for(int i = 0; i < numArgs; i++){
		strcat(final, " ");
		strcat(final, args[i]);
	}
	strcat(final, "\n");
}

void reg(char *op, char args[2][MAX_INPUT_SIZE]) {
	int numTokens, errFlag = 0;
	char finalCommand[3*MAX_INPUT_SIZE], command_args[MAX_INPUT_SIZE] = "";
	concatenateArgs(command_args, 2, args);
	strcpy(finalCommand, "REG");
	strcat(finalCommand, command_args);

	for(int i = 0; i < 5; i ++){
		if(isalpha(args[0][i]) != 0){
			printf("UID must contain numbers only.\n");
			errFlag = 1;
			break;
		}
	}
	if(strlen(args[0]) != 5){
		printf("UID must have 5 numbers.\n");
		errFlag = 1;
	}
	if(strlen(args[1]) != 8){
		printf("Password must have 8 characters.\n");
		errFlag = 1;
	}

	for(int i = 0; i < 8; i ++){
		if(isalpha(args[1][i]) != 0 && isdigit(args[1][i]) != 0) {
			printf("Password must contain alphanumeric characters only.\n");
			errFlag = 1;
			break;
		}
	}

	if(errFlag) return;

	n = sendto(udpSocket, finalCommand, strlen(finalCommand), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if(n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if(n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", op, args[0]);
	if(numTokens != 2 || strcmp(op, "RRG") != 0){
		fprintf(stdout, "Server Error.\n");
		exit(1);
	} 
	else if(strcmp(args[0], "OK") == 0) fprintf(stdout, "User Successfully Registered.\n");
	else if(strcmp(args[0], "DUP") == 0) fprintf(stdout, "User Already Registered.\n");
	else if(strcmp(args[0], "NOK") == 0) fprintf(stdout, "User Not Registered.\n");
	else exit(1);
}

void unr(char *op, char args[2][MAX_INPUT_SIZE]) {
	int numTokens, errFlag = 0;
	char finalCommand[3*MAX_INPUT_SIZE], command_args[MAX_INPUT_SIZE] = "";
	concatenateArgs(command_args, 2, args);
	strcpy(finalCommand, "UNR");
	strcat(finalCommand, command_args);

	for(int i = 0; i < 5; i ++){
		if(isalpha(args[0][i]) != 0){
			printf("UID must contain numbers only.\n");
			errFlag = 1;
			break;
		}
	}
	if(strlen(args[0]) != 5){
		printf("UID must have 5 numbers.\n");
		errFlag = 1;
	}
	if(strlen(args[1]) != 8){
		printf("Password must have 8 characters.\n");
		errFlag = 1;
	}

	for(int i = 0; i < 8; i ++){
		if(isalpha(args[1][i]) != 0 && isdigit(args[1][i]) != 0) {
			printf("Password must contain alphanumeric characters only.\n");
			errFlag = 1;
			break;
		}
	}

	if(errFlag) return;

	n = sendto(udpSocket, finalCommand, strlen(finalCommand), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if(n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if(n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", op, args[0]);
	if(numTokens != 2 || strcmp(op, "RUN") != 0){
		fprintf(stdout, "Server Error.\n");
		exit(1);
	} 
	else if(strcmp(args[0], "OK") == 0) fprintf(stdout, "User Successfully Unregistered.\n");
	else if(strcmp(args[0], "NOK") == 0) fprintf(stdout, "User Not Unregistered.\n");
	else exit(1);
}

void readCommands() {
	while (fgets(buffer, sizeof(buffer)/sizeof(char), stdin)) {
		char op[MAX_INPUT_SIZE], args[2][MAX_INPUT_SIZE];

        int numTokens = sscanf(buffer, "%s %s %s", op, args[0], args[1]);

		if(strcmp(op, "quit") == 0) exit(1);
        if (numTokens < 1) fprintf(stderr, "error: incorrect command line arguments\n");
		else {
			if(strcmp(op, "reg") == 0) {
				reg(op, args);
			}
			if(strcmp(op, "unr") == 0 || strcmp(op, "unregister") == 0) {
				unr(op, args);
			}
		}
	}
}

int main(int argc, char *argv[]) {
	parseArgs(argc, argv);
	createSockets();
	readCommands();
	deleteSockets();

	return 0;
}