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

int verifyDigit(char buff[], int beg, int end, char err[]) {
	for (int i = beg; i < end; i ++){
		if (isdigit(buff[i]) == 0) {
			fprintf(stderr, "error: %s\n", err);
			return 1;
		}
	}
	return 0;
}

int verifyAlnum(char buff[], int beg, int end, char err[]) {
	for (int i = beg; i < end; i ++){
		if (isalnum(buff[i]) == 0) {
			fprintf(stderr, "error: %s\n", err);
			return 1;
		}
	}
	return 0;
}

int verifyAlpha(char buff[], int beg, int end, char err[]) {
	for (int i = beg; i < end; i ++){
		if (isalpha(buff[i]) == 0) {
			fprintf(stderr, "error: %s \n", err);
			return 1;
		}
	}
	return 0;
}

int verifyUserInfo(char uid[], char pwd[]) {
	int errFlag = 0;
	errFlag = verifyDigit(uid, 0, strlen(uid), "error: UID must contain numbers only");

	if (strlen(uid) != 5){
		fprintf(stderr, "error: UID must have 5 numbers\n");
		errFlag = 1;
	}
	if (strlen(pwd) != 8){
		fprintf(stderr, "error: Password must have 8 characters\n");
		errFlag = 1;
	}
	errFlag = verifyAlnum(pwd, 0, strlen(pwd), "Password must contain alphanumeric characters only");

	return errFlag;
}

int verifySession() {
	if (!loggedUser.logged) {
		fprintf(stdout, "warning: No user logged.\n");
		return 0;
	}
	if (!selectedGroup.selected) {
		fprintf(stdout, "warning: No group selected.\n");
		return 0;
	}
	return 1;
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
