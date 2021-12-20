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

typedef struct user {
	int logged;
	char uid[6];
	char pwd[9];
} user;

int udpSocket, tcpSocket, errcode, errno;
struct addrinfo hints, *udpRes, *tcpRes;
ssize_t n;
socklen_t addrlen;
struct sockaddr_in addr;

char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_INPUT_SIZE]; //nosso pc: localhost;
user loggedUser; 

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

void concatenateArgs(char *final, char args[][MAX_INPUT_SIZE], int argsNumber) {
	for(int i = 0; i < argsNumber; i++){
		strcat(final, " ");
		strcat(final, args[i]);
	}
	strcat(final, "\n");
}

void reg() {
	int numTokens, errFlag = 0;
	char args[2][MAX_INPUT_SIZE], command[MAX_INPUT_SIZE] = "REG", argsCommand[MAX_INPUT_SIZE] = "";

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);

	if (numTokens != 2) fprintf(stderr, "error: incorrect command line arguments\n");
	for (int i = 0; i < 5; i ++){
		if(isalpha(args[0][i]) != 0){
			fprintf(stderr, "error: UIDSIZE] must contain numbers only\n");
			errFlag = 1;
			break;
		}
	}
	if (strlen(args[0]) != 5){
		fprintf(stderr, "error: UID must have 5 numbers\n");
		errFlag = 1;
	}
	if (strlen(args[1]) != 8){
		fprintf(stderr, "error: Password must have 8 characters\n");
		errFlag = 1;
	}
	for (int i = 0; i < 8; i ++){
		if(isalpha(args[1][i]) != 0 && isdigit(args[1][i]) != 0) {
			fprintf(stderr, "error: Password must contain alphanumeric characters only\n");
			errFlag = 1;
			break;
		}
	}
	if(errFlag) return;

	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if(n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if(n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
	if(numTokens != 2 || strcmp(args[0], "RRG") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if(strcmp(args[1], "OK") == 0) fprintf(stdout, "User successfully registered.\n");
	else if(strcmp(args[1], "DUP") == 0) fprintf(stdout, "User already registered.\n");
	else if(strcmp(args[1], "NOK") == 0) fprintf(stdout, "User not registered.\n");
	else exit(1);
}

void unr() {
	int numTokens, errFlag = 0;
	char args[2][MAX_INPUT_SIZE], command[MAX_INPUT_SIZE] = "UNR", argsCommand[MAX_INPUT_SIZE] = "";

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);

	if (numTokens != 2) fprintf(stderr, "error: incorrect command line arguments\n");
	for (int i = 0; i < 5; i ++){
		if(isalpha(args[0][i]) != 0){
			fprintf(stderr, "error: UID must contain numbers only\n");
			errFlag = 1;
			break;
		}
	}
	if (strlen(args[0]) != 5){
		fprintf(stderr, "error: UID must have 5 numbers\n");
		errFlag = 1;
	}
	if (strlen(args[1]) != 8){
		fprintf(stderr, "error: Password must have 8 characters\n");
		errFlag = 1;
	}
	for (int i = 0; i < 8; i ++){
		if(isalpha(args[1][i]) != 0 && isdigit(args[1][i]) != 0) {
			fprintf(stderr, "error: Password must contain alphanumeric characters only\n");
			errFlag = 1;
			break;
		}
	}
	if(errFlag) return;

	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if(n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if(n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
	if(numTokens != 2 || strcmp(args[0], "RUN") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if(strcmp(args[1], "OK") == 0) fprintf(stdout, "User successfully unregistered.\n");
	else if(strcmp(args[1], "NOK") == 0) fprintf(stdout, "User not unregistered.\n");
	else exit(1);
}

void resetUser() {
	loggedUser.logged = 0;
	strcpy(loggedUser.uid, "");
	strcpy(loggedUser.pwd, "");
}

void login() {
	int numTokens, errFlag = 0;
	char args[2][MAX_INPUT_SIZE], command[MAX_INPUT_SIZE] = "LOG", argsCommand[MAX_INPUT_SIZE] = "";

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);

	if (numTokens != 2) fprintf(stderr, "error: incorrect command line arguments\n");
	for (int i = 0; i < 5; i ++){
		if(isalpha(args[0][i]) != 0){
			fprintf(stderr, "error: UID must contain numbers only\n");
			errFlag = 1;
			break;
		}
	}
	if (strlen(args[0]) != 5){
		fprintf(stderr, "error: UID must have 5 numbers\n");
		errFlag = 1;
	}
	if (strlen(args[1]) != 8){
		fprintf(stderr, "error: Password must have 8 characters\n");
		errFlag = 1;
	}
	for (int i = 0; i < 8; i ++){
		if(isalpha(args[1][i]) != 0 && isdigit(args[1][i]) != 0) {
			fprintf(stderr, "error: Password must contain alphanumeric characters only\n");
			errFlag = 1;
			break;
		}
	}
	if(errFlag) return;

	strcpy(loggedUser.uid, args[0]);
	strcpy(loggedUser.pwd, args[1]);

	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if(n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if(n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
	if(numTokens != 2 || strcmp(args[0], "RLO") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if(strcmp(args[1], "OK") == 0) {
		fprintf(stdout, "User successfully logged in.\n");
		loggedUser.logged = 1;
	}
	else if(strcmp(args[1], "NOK") == 0) {
		fprintf(stdout, "User not logged in.\n");
		resetUser();
	}
	else exit(1);
}

void logout() {
	int numTokens;
	char args[2][MAX_INPUT_SIZE], command[MAX_INPUT_SIZE] = "OUT", argsCommand[MAX_INPUT_SIZE] = "";

	strcpy(args[0], loggedUser.uid);
	strcpy(args[1], loggedUser.pwd);
	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if(n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if(n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
	if(numTokens != 2 || strcmp(args[0], "ROU") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if(strcmp(args[1], "OK") == 0) {
		fprintf(stdout, "User successfully logged out.\n");
		resetUser();
	}
	else if(strcmp(args[1], "NOK") == 0) fprintf(stdout, "User not logged out.\n");
	else exit(1);
}

void su() {
	if (loggedUser.logged) fprintf(stdout, "UID of logged user: %s.\n", loggedUser.uid);
	else fprintf(stdout, "No logged user.\n");
}

void exit() {
	fprintf(stdout, "Terminating user application.\n");
	resetUser();
	deleteSockets();
}

void readCommands() {
	resetUser();
	
	while (fgets(buffer, sizeof(buffer)/sizeof(char), stdin)) {
		char op[MAX_INPUT_SIZE];

        int numTokens = sscanf(buffer, "%s %[^\n]", op, buffer);

        if (numTokens < 1) fprintf(stderr, "error: incorrect command line arguments\n");
		else {
			if(strcmp(op, "reg") == 0) {
				reg();
			}
			else if(strcmp(op, "unr") == 0 || strcmp(op, "unregister") == 0) {
				unr();
			}
			else if(strcmp(op, "login") == 0) {
				login();
			}
			else if(strcmp(op, "logout") == 0) {
				logout();
			}
			else if(strcmp(op, "su") == 0 || strcmp(op, "showuid") == 0) {
				su();
			}
			else if(strcmp(op, "exit") == 0) {
				return;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	parseArgs(argc, argv);
	createSockets();
	readCommands();
	exit();

	return 0;
}