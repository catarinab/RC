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
	else exit(1);
}

void post() {
	int numTokens, lenMessage;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "PST ", *len;

	if (!(verifySession())) return;

	numTokens = sscanf(buffer, "\"%[^\"]\" %s", buffer, args[0]);
	if (numTokens < 1) {
		fprintf(stderr, "error: Incorrect command line arguments\n");
		return;
	}
	createTCPSocket();

    asprintf(&len, "%d", (lenMessage = strlen(buffer)));
	strcat(strcat(strcat(strcat(strcat(strcat(command, loggedUser.uid), " "), selectedGroup.gid), " "), len), " ");
	free(len);
	sendTCPMessage(tcpSocket, command, strlen(command));
	sendTCPMessage(tcpSocket, buffer, lenMessage);
	memset(buffer, 0, MAX_INPUT_SIZE);

	if (numTokens == 2) {
		FILE *ptr;
		int lenFname, lenFile;
		
		if (verifyAlnum(args[0], 0, (lenFname = strlen(args[0])) - 5, "Fname must contain alphanumeric characters only") || verifyAlpha(args[0], lenFname - 3, lenFname, "Fname must contain a valid extension")) {
			close(tcpSocket);
			return;
		}
		else if (lenFname < 5 || args[0][lenFname - 4] != '.') {
			fprintf(stderr, "error: Fname must contain a valid extension.\n");
			close(tcpSocket);
			return;
		}

		if (!(ptr = fopen(args[0], "rb"))) {
			fprintf(stderr, "error: File does not exist.\n");
			close(tcpSocket);
			return;
		}
		fseek(ptr, 0L, SEEK_END);
		lenFile = ftell(ptr);
		fseek(ptr, 0L, SEEK_SET);
		asprintf(&len, "%d", lenFile);
		strcat(strcat(strcat(strcat(strcpy(command, " "), args[0]), " "), len), " ");
		free(len);

		sendTCPMessage(tcpSocket, command, strlen(command));

		while (0 < (lenFile = fread(buffer, sizeof(char), MAX_INPUT_SIZE, ptr))) {
			sendTCPMessage(tcpSocket, buffer, lenFile);
			memset(buffer, 0, MAX_INPUT_SIZE);
		}
	}
	sendTCPMessage(tcpSocket, "\n", 1);

	receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
	close(tcpSocket);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
	if (numTokens < 2 || strcmp(args[0], "RPT") != 0) fprintf(stderr, "error: Server Error.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "The message was not posted.\n");
	else fprintf(stdout, "The message was successfully posted with MID %s.\n", args[1]);
}

void ret() {
	int numTokens;
	char args[3][MAX_INFO], command[MAX_COMMAND_SIZE] = "RTV ";

	if (!(verifySession())) return;

	numTokens = sscanf(buffer, "%s", args[0]);
	if (numTokens < 1) {
		fprintf(stderr, "error: Incorrect command line arguments\n");
		return;
	}
	else if (strlen(args[0]) > 4) {
		fprintf(stderr, "error: MID contains too many characters\n");
		return;
	}
	else if (verifyDigit(args[0], 0, strlen(args[0]), "MID must contain numeric characters only")) return;
	createTCPSocket();

	strcat(strcat(command, args[0]), "\n");
	sendTCPMessage(tcpSocket, command, strlen(command));
}
