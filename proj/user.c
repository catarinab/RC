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
#include "header/udpRequests.h"
#include "header/tcpRequests.h"
// login 84745 00000000

void parseArgs(int argc, char *argv[]) {
	if (argc == 1) {
		if (gethostname(ip, 128) == -1) fprintf(stderr, "error: %s\n", strerror(errno));
		strcpy(port, "58056");
	}
	else if (argc == 5) {
		if (strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-p") == 0) {
			strcpy(ip, argv[2]);
			strcpy(port, argv[4]);
		} 
		else if (strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-n") == 0) {
			strcpy(port, argv[2]);
			strcpy(ip, argv[4]);
		} 
		else fprintf(stderr, "error: incorrect command line arguments\n");
	}
	else if (argc == 3) {
		if (strcmp(argv[1], "-n") == 0) strcpy(ip, argv[2]);
		else if (strcmp(argv[1], "-p") == 0) strcpy(port, argv[2]);
		else fprintf(stderr, "error: incorrect command line arguments\n");
	}
	else fprintf(stderr, "error: incorrect command line arguments\n");
}


void deleteSockets() {
	freeaddrinfo(udpRes);
	freeaddrinfo(tcpRes);
	close(udpSocket);
	close(tcpSocket);
}

void readCommands() {
	resetUser();
	resetGroup();
	
	while (fgets(buffer, sizeof(buffer)/sizeof(char), stdin)) {
		char op[MAX_OP_SIZE];

        int numTokens = sscanf(buffer, "%s %[^\n]", op, buffer);

        if (numTokens < 1) fprintf(stderr, "error: incorrect command line arguments\n");
		else {
			if (strcmp(op, "reg") == 0) {
				reg();
			}
			else if (strcmp(op, "unr") == 0 || strcmp(op, "unregister") == 0) {
				unr();
			}
			else if (strcmp(op, "login") == 0) {
				login();
			}
			else if (strcmp(op, "logout") == 0) {
				logout();
			}
			else if (strcmp(op, "su") == 0 || strcmp(op, "showuid") == 0) {
				su();
			}
			else if (strcmp(op, "exit") == 0) {
				return;
			}
			else if (strcmp(op, "gl") == 0 || strcmp(op, "groups") == 0) {
				gl();
			}
			else if (strcmp(op, "s") == 0 || strcmp(op, "subscribe") == 0) {
				sub();
			}
			else if (strcmp(op, "u") == 0 || strcmp(op, "unsubscribe") == 0) {
				unsub();
			}
			else if (strcmp(op, "mgl") == 0 || strcmp(op, "my_groups") == 0) {
				mgl();
			}
			else if (strcmp(op, "sag") == 0 || strcmp(op, "select") == 0) {
				sag();
			}
			else if (strcmp(op, "sg") == 0 || strcmp(op, "showgid") == 0) {
				sg();
			}
			else if(strcmp(op, "ulist") == 0 || strcmp(op, "ul") == 0) {
				ul();
			}
			else if(strcmp(op, "post") == 0) {
				post();
			}
			else if(strcmp(op, "r") == 0 || strcmp(op, "retrieve") == 0) {
				ret();
			}
			else fprintf(stdout, "Operation not recognized.\n");
		}
	}
}

void exitClientSession() {
	fprintf(stdout, "Terminating user application.\n");
	resetUser();
	resetGroup();
	deleteSockets();
}

int main(int argc, char *argv[]) {
	parseArgs(argc, argv);
	createUdpSocket();
	readCommands();
	exitClientSession();
	return 0;
}