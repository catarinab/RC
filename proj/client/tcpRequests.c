#define _GNU_SOURCE
#include "header/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct stat st = {0};

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
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE];
	char responseBuffer[MAX_USER_SUB], groupName[MAX_INFO]; // fazer as contas do tamanho
	if (!selectedGroup.selected) {
		fprintf(stdout, "warning: No group selected.\n");
		return;
	}
	createTCPSocket();

	sprintf(command, "ULS %s\n", selectedGroup.gid);
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
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE], *len;

	if (!(verifySession())) return;

	numTokens = sscanf(buffer, " \"%[^\"]\" %s", buffer, args[0]);
	if (numTokens < 1) {
		fprintf(stderr, "error: Incorrect command line arguments\n");
		return;
	}
	createTCPSocket();

    asprintf(&len, "%d", (lenMessage = strlen(buffer)));
	sprintf(command, "PST %s %s %s ", loggedUser.uid, selectedGroup.gid, len);
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
		memset(command, 0, MAX_COMMAND_SIZE);
		sprintf(command, " %s %s ", args[0], len);
		free(len);

		sendTCPMessage(tcpSocket, command, strlen(command));

		while (0 < (lenFile = fread(buffer, sizeof(char), MAX_INPUT_SIZE, ptr))) {
			sendTCPMessage(tcpSocket, buffer, lenFile);
			memset(buffer, 0, MAX_INPUT_SIZE);
		}
		fclose(ptr);
	}
	sendTCPMessage(tcpSocket, "\n", 1);

	receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
	close(tcpSocket);

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);
	if (numTokens < 2 || strcmp(args[0], "RPT") != 0) fprintf(stderr, "error: Server Error.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "The message was not posted.\n");
	else fprintf(stdout, "The message was successfully posted with MID %s.\n", args[1]);
}

char * movePointer(char *table, int tableSize , char *pointer, int *totalShifts, int shift) {
	if ((*totalShifts += shift) < tableSize) {
		pointer = pointer + shift * sizeof(char);
	}
	else {
		*totalShifts -= tableSize;
		receiveTCPMessage(tcpSocket, table, tableSize);
		pointer = table + (*totalShifts) * sizeof(char);
	}
	return pointer;
}

void ret() {
	int numTokens, n, messageSize, fileSize, totalShifts = 0, shift;
	char args[3][MAX_INFO], command[MAX_COMMAND_SIZE], message[MAX_MESSAGE_SIZE], directory[MAX_INFO] = "Groups/", fileName[MAX_INFO], *bufferPointer;
	FILE *ptr;

	if (!(verifySession())) return;
	if (stat(directory, &st) == -1)
		mkdir(directory, 0777);
	strcat(strcat(directory, selectedGroup.gid), "/");

	numTokens = sscanf(buffer, " %s", args[0]);
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

	if ((n = 4 - strlen(args[0])) > 0) {
		for (int i = 4 - n; i >= 0; i--) {
			args[0][i + n] = args[0][i];
		}
		for (int i = 0; i < n; i++) {
			args[0][i] = '0';
		}
	}

	sprintf(command, "RTV %s %s %s\n", loggedUser.uid, selectedGroup.gid, args[0]);
	memset(buffer, 0, MAX_INPUT_SIZE);
	sendTCPMessage(tcpSocket, command, strlen(command));
	receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
	
	numTokens = sscanf(buffer, " %s %s ", args[0], args[1]);
	shift = strlen(args[0]) + strlen(args[1]) + 2;
	bufferPointer = buffer;
	bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
	if (numTokens < 2 || strcmp(args[0], "RRT") != 0) fprintf(stderr, "error: Server Error.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "There was a problem with the retrieve request.\n");
	else if (strcmp(args[1], "EOF") == 0) fprintf(stdout, "There are no messages available.\n");
	else if (strcmp(args[1], "OK") == 0) {
		numTokens = sscanf(bufferPointer, "%s ", args[0]);
		if (numTokens < 1) fprintf(stderr, "error: Server Error.\n");
		shift = strlen(args[0]) + 1;
		bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
		n = atoi(args[0]);
		for (int i = 0; i < n; i++) {
			numTokens = sscanf(bufferPointer, "%s %s %s ", args[0], args[1], args[2]);
			if (numTokens < 3) {
				receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
				if (numTokens == 0) {
					numTokens = sscanf(buffer, " %s %s %s ", args[0], args[1], args[2]);
					shift = strlen(args[0]) + strlen(args[1]) + strlen(args[2]) + 3;
				}
				else if (numTokens == 1) {
					numTokens = sscanf(buffer, " %s %s ", args[1], args[2]);
					shift = strlen(args[1]) + strlen(args[2]) + 2;
				}
				else {
					numTokens = sscanf(buffer, " %s ", args[2]);
					shift =  strlen(args[2]) + 1;
				}
				totalShifts = 0;
				bufferPointer = buffer;
				bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
			}
			else {
				shift = strlen(args[0]) + strlen(args[1]) + strlen(args[2]) + 3;
				bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
			}
			messageSize = atoi(args[2]);
			memset(message, 0, MAX_MESSAGE_SIZE);
			if (messageSize > MAX_INPUT_SIZE - totalShifts) {
				strcpy(message, bufferPointer);
				receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
				strncpy(&message[strlen(message)], buffer, (shift = messageSize - strlen(message)) - 1);
				totalShifts = 0;
				bufferPointer = buffer;
				bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift + 1);
			}
			else {
				strncpy(message, bufferPointer, messageSize - 1);
				bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, messageSize + 1);
			}
			fprintf(stdout, "Message of MID %s, posted by user with UID %s: \"%s\"", args[0], args[1], message);
			if (bufferPointer[0] == '/') {
				numTokens = sscanf(bufferPointer, "/ %s %s ", args[0], args[1]);
				if (numTokens < 2) {
					receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
					if (buffer[0] == '/') shift = 2;
					else if (buffer[0] == ' ') shift = 1;

					if (numTokens == 0) {
						numTokens = sscanf(buffer, " / %s %s ", args[0], args[1]);
						shift += strlen(args[0]) + strlen(args[1]) + 2;
					}
					else {
						numTokens = sscanf(buffer, "  %s ", args[1]);
						shift +=  strlen(args[1]) + 1;
					}
					totalShifts = 0;
					bufferPointer = buffer;
					bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
				}
				else {
					shift = strlen(args[0]) + strlen(args[1]) + 4;
					bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
				}
				fileSize = atoi(args[1]);
				if (stat(directory, &st) == -1)
					mkdir(directory, 0777);
				strcat(strcpy(fileName, directory), args[0]);
				fprintf(stdout, " (associated with the file %s)", fileName);
				if (!(ptr = fopen(fileName, "wb"))) {
					fprintf(stderr, "error: Can't create the file %s.\n", fileName);
					close(tcpSocket);
					return;
				}
				while (fileSize > 0) {
					if (fileSize >= MAX_INPUT_SIZE - totalShifts) {
						if (fwrite(bufferPointer, sizeof(char), (shift = MAX_INPUT_SIZE - totalShifts), ptr) != shift) {
							fprintf(stderr, "error: Can't write to the file %s.\n", fileName); 
							exit(1);
						}
						fileSize -= shift;
						receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
						totalShifts = 0;
						bufferPointer = buffer;
					}
					else {
						if (fwrite(bufferPointer, sizeof(char), fileSize, ptr) != fileSize) {
							fprintf(stderr, "error: Can't write to the file %s.\n", fileName); 
							exit(1);
						}
						bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, fileSize + 1);
						fileSize -= fileSize;
					}
				}
				fclose(ptr);
			}
			fprintf(stdout, "\n");
		}
	} 
	else exit(1);
}
