#define _GNU_SOURCE
#include "header/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

void createTCPSocket() {
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpSocket == -1) exit(1);

	memset(&tcpHints, 0, sizeof(tcpHints));
	tcpHints.ai_family = AF_INET;
	tcpHints.ai_socktype = SOCK_STREAM;

	errcode = getaddrinfo(ip, port, &tcpHints, &tcpRes);
	if (errcode == -1) exit(1);
	n=connect(tcpSocket,tcpRes->ai_addr,tcpRes->ai_addrlen);
	if(n==-1)exit(1);
}

void sendTCPMessage(int socket, char *ptr, int nleft) {
	while (nleft > 0) {
		n = write(socket, ptr, nleft);
		if (n == -1) exit(1);
		else if (n == 0) break;
		nleft -= n;
		ptr += n;
	}
}

void receiveTCPMessage(int socket, char *ptr, int nleft) {
	while (nleft > 0) {
		n = read(socket, ptr, nleft);
		printf("%s", buffer);
		if (n == -1) exit(1);
		else if (n == 0) break;
		nleft -= n;
		ptr += n;
	}
}

void ul() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "ULS ";
	char responseBuffer[MAX_USER_SUB], groupName[MAX_INFO]; // fazer as contas do tamanho
	if (!selectedGroup.selected) {
		fprintf(stdout, "warning: No group selected.\n");
		return;
	}
	createTCPSocket();

	strcat(command, selectedGroup.gid);
	strcat(command, "\n");

	sendTCPMessage(tcpSocket, command, strlen(command));

	memset(responseBuffer, 0, MAX_USER_SUB);
	receiveTCPMessage(tcpSocket, responseBuffer, MAX_USER_SUB);
	close(tcpSocket);

	numTokens = sscanf(responseBuffer, "%s %s %s%[^\n]", args[0], args[1], groupName, responseBuffer);
	if (numTokens < 2 || strcmp(args[0], "RUL") != 0) fprintf(stderr, "error: Server Error.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "The group does not exist.\n");
	else if (strcmp(args[1], "OK") == 0){
		if(numTokens == 3) fprintf(stdout, "No Users subscribed to group %s.\n", groupName);
		else fprintf(stdout, "Users subscribed to group %s:%s\n", groupName, responseBuffer);
	} 
	else fprintf(stderr, "error: Server Error.\n");
}

void post() {
	loggedUser.logged = 1;
	strcpy(loggedUser.uid, "93230");
	selectedGroup.selected = 1;
	strcpy(selectedGroup.gid, "28");
	int numTokens, lenMessage, nleft;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "PST ", *len;

	if (!loggedUser.logged) {
		fprintf(stdout, "warning: No user logged.\n");
		return;
	}
	if (!selectedGroup.selected) {
		fprintf(stdout, "warning: No group selected.\n");
		return;
	}

	numTokens = sscanf(buffer, "\"%[^\"]\" %s", buffer, args[0]);
	if (numTokens < 1) fprintf(stderr, "error: incorrect command line arguments\n");
	createTCPSocket();

	strcat(command, loggedUser.uid);
	strcat(command, " ");
	strcat(command, selectedGroup.gid);
	strcat(command, " ");
    asprintf(&len, "%d", (lenMessage = strlen(buffer)));
    strcat(command, len);
	free(len);
	strcat(command, " ");

	sendTCPMessage(tcpSocket, command, strlen(command));
	sendTCPMessage(tcpSocket, buffer, lenMessage);

	if (numTokens == 2) {
		int errFlag = 0, lenFname;
		for (int i = 0; i < (lenFname = strlen(args[0])) - 5; i ++) {
			if (isalnum(args[0][i]) == 0 && args[0][i] != '.' && args[0][i] != '-' && args[0][i] != '_') {
				fprintf(stderr, "error: Fname must contain alphanumeric characters only\n");
				errFlag = 1;
				break;
			}
		}
		if (lenFname < 5 || args[0][lenFname - 4] != '.') {
			fprintf(stderr, "error: Fname must contain a valid extension.\n");
			errFlag = 1;
		}
		for (int i = lenFname - 3; i < lenFname; i ++){
			if (isalpha(args[0][i]) == 0) {
				fprintf(stderr, "error: Fname must contain a valid extension.\n");
				errFlag = 1;
				break;
			}
		}
		if (errFlag) {
			close(tcpSocket);
			return;
		}

		strcpy(command, " ");
		sendTCPMessage(tcpSocket, command, 1);
		
		FILE *ptr;
		int lenFile;
		ptr = fopen(args[0], "rb");

		strcpy(command, args[0]);
		strcat(command, " ");
		fseek(ptr, 0L, SEEK_END);
		lenFile = ftell(ptr);
		fseek(ptr, 0L, SEEK_SET);
		asprintf(&len, "%d", lenFile);
    	strcat(command, len);
		free(len);
		strcat(command, " ");

		sendTCPMessage(tcpSocket, command, strlen(command));

		int fileSize;
		memset(buffer, 0, MAX_INPUT_SIZE);
		while (0 < (fileSize = fread(buffer, sizeof(char), MAX_INPUT_SIZE, ptr))) {
			sendTCPMessage(tcpSocket, buffer, fileSize);
			memset(buffer, 0, MAX_INPUT_SIZE);
		}
	}
	strcpy(command, "\n");
	sendTCPMessage(tcpSocket, command, 1);

	memset(buffer, 0, MAX_INPUT_SIZE);
	receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
	close(tcpSocket);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
	if (numTokens < 2 || strcmp(args[0], "RPT") != 0) fprintf(stderr, "error: Server Error.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "The message was not posted.\n");
	else fprintf(stdout, "The message was successfully posted with MID %s.\n", args[1]);
}
