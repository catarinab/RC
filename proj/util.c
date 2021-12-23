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
#include "header/constants.h"

int udpSocket, tcpSocket, errcode, errno;
struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
ssize_t n;
socklen_t addrlen;
struct sockaddr_in addr;

char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_IP_SIZE]; //nosso pc: localhost;
user loggedUser; 
group selectedGroup;

void concatenateArgs(char *final, char args[][MAX_INFO], int argsNumber) {
	for(int i = 0; i < argsNumber; i++){
		strcat(final, " ");
		strcat(final, args[i]);
	}
	strcat(final, "\n");
}

int verifyUserInfo(char uid[], char pwd[]) {
	int errFlag = 0;
	for (int i = 0; i < strlen(uid); i ++){
		if (isdigit(uid[i]) == 0){
			fprintf(stderr, "error: UID must contain numbers only\n");
			errFlag = 1;
			break;
		}
	}
	if (strlen(uid) != 5){
		fprintf(stderr, "error: UID must have 5 numbers\n");
		errFlag = 1;
	}
	if (strlen(pwd) != 8){
		fprintf(stderr, "error: Password must have 8 characters\n");
		errFlag = 1;
	}
	for (int i = 0; i < strlen(pwd); i ++){
		if (isalnum(pwd[i]) == 0) {
			fprintf(stderr, "error: Password must contain alphanumeric characters only\n");
			errFlag = 1;
			break;
		}
	}
	return errFlag;
}

void resetUser() {
	loggedUser.logged = 0;
	strcpy(loggedUser.uid, "");
	strcpy(loggedUser.pwd, "");
}

void resetGroup() {
	selectedGroup.selected = 0;
	strcpy(selectedGroup.gid, "");
}
