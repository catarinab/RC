#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include<dirent.h>
#include "header/util.h"

void createTcpSocket() {
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1) exit(1);

    memset(&tcpHints, 0, sizeof(tcpHints));
    tcpHints.ai_family = AF_INET;
    tcpHints.ai_socktype = SOCK_STREAM;
    tcpHints.ai_flags = AI_PASSIVE;

	errcode = getaddrinfo(NULL, port, &tcpHints, &tcpRes);
	if (errcode == -1) exit(1);

    n = bind(tcpSocket, tcpRes->ai_addr, tcpRes->ai_addrlen);
    if (n == -1) exit(1);

    if (listen(tcpSocket, 5) == -1) exit(1);
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
		else if (n == 0 || ptr[n - 1] == '\n'){
			ptr[n] = '\0';
			break; 
		}
		nleft -= n;
		ptr += n;
	}
}

void uls() {
	DIR *d;
	FILE *ptr;
    struct dirent *dir;
	int numTokens;
    char args[1][MAX_INFO], reply[30], pathname[20], gname[MAX_INFO], uid[MAX_INFO];

    numTokens = sscanf(buffer, "%s", args[0]);
	memset(buffer, 0, MAX_INPUT_SIZE);
    strcpy(buffer, "RUL ");
    if (numTokens != 1) strcat(buffer, "NOK\n");
	else if (strcmp(args[0], "00") == 0) strcat(buffer, "NOK\n");
    else if (!(checkGroup(args[0]))) strcat(buffer, "NOK\n");
    else {
		sprintf(pathname, "GROUPS/%s/%s_name.txt", args[0], args[0]);
		memset(gname, 0, MAX_INFO);
		if (!(ptr = fopen(pathname, "r"))) exit(1);
		if (0 >= fread(gname, sizeof(char), MAX_INFO, ptr)) exit(1);
		fclose(ptr);
		sprintf(reply, "OK %s", gname);
		strcat(buffer, reply);
		memset(pathname, 0, 20);
		sprintf(pathname, "GROUPS/%s", args[0]);
		d = opendir(pathname);
		if (d) {
			while ((dir = readdir(d)) != NULL) {
				if (strlen(dir->d_name) == 9 && verifyUserFile(dir->d_name, uid)) {
					memset(reply, 0, MAX_INFO);
					sprintf(reply, " %s", uid);
					strcat(buffer, reply);
				}
			}
			closedir(d);
			strcat(buffer, "\n");
		}
		else exit(1);
    }

    if (mode == verbose) fprintf(stdout, "ULS, GID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    sendTCPMessage(newTcpSocket, buffer, strlen(buffer));
}

char * movePointer(char *table, int tableSize , char *pointer, int *totalShifts, int shift) {
	printf("wtf\n");
	if ((*totalShifts += shift) < tableSize) {
		pointer = pointer + shift * sizeof(char);
	}
	else {
		*totalShifts -= tableSize;
		receiveTCPMessage(newTcpSocket, table, tableSize);
		pointer = table + (*totalShifts) * sizeof(char);
	}
	printf("wtf2\n");
	return pointer;
}

void pst() {
	FILE *ptr;
	int numTokens, msgSize, fileSize, shift, totalShifts, errFlag = 0;
    char args[2][MAX_INFO], uid[6], gid[3], mid[5], reply[REPLY_SIZE] = "RPT ", message[MAX_MESSAGE_SIZE], pathname[45], *bufferPointer = buffer;

	printf("OI\n");
	printf("%s\n", bufferPointer);
	numTokens = sscanf(bufferPointer, "%s %s ", args[0], args[1]);
	printf("lol\n");
	shift = strlen(args[0]) + strlen(args[1]) + 2;
	printf("oui oui baguette\n");
	bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
	printf("????\n");
	if (numTokens != 2) strcat(reply, "NOK\n");
	else if (!(checkUser(args[0]))) strcat(reply, "NOK\n");
	else if (!(checkLog(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkGroup(args[1]))) strcat(reply, "NOK\n");
	else if (!(checkMessage(args[1], mid))) strcat(reply, "NOK\n");
    else {
		printf("OI! %s\n", mid);
		strcpy(uid, args[0]);
		memset(args[0], 0, MAX_INFO);
		strcpy(gid, args[1]);
		memset(args[1], 0, MAX_INFO);
		printf("%s\n", bufferPointer);
		numTokens = sscanf(bufferPointer, "%s ", args[0]);
		shift = strlen(args[0]) + 1;
		bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
		if (numTokens != 1) strcat(reply, "NOK\n");
		else {
			printf("OI?\n");
			msgSize = atoi(args[0]);
			printf("msgSize: %d\n", msgSize);
			memset(message, 0, MAX_MESSAGE_SIZE);
			strncpy(message, bufferPointer, msgSize);
			printf("%s\n", bufferPointer);
			bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, msgSize);
			if (!(createMsgDir(uid, gid, mid, message))) strcat(reply, "NOK\n");
			else {
				memset(args[0], 0, MAX_INFO);
				memset(args[1], 0, MAX_INFO);
				printf("%s\n", bufferPointer);
				numTokens = sscanf(bufferPointer, " %s %s ", args[0], args[1]);
				printf("nTokens: %d\n", numTokens);
				shift = strlen(args[0]) + strlen(args[1]) + 2;
				bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
				if (numTokens == -1) strcat(strcat(reply, mid), "\n");
				else if (numTokens =! 2) strcat(reply, "NOK\n");
				else {
					printf("ENTREI\n");
					fileSize = atoi(args[1]);
					sprintf(pathname, "GROUPS/%s/MSG/%s/%s", gid, mid, args[0]);
					if (!(ptr = fopen(pathname, "w"))) strcat(reply, "NOK\n");
					else {
						while (fileSize > 0) {
							printf("LENDO\n");
							if (fileSize > MAX_INPUT_SIZE - totalShifts) {
								if (fwrite(bufferPointer, sizeof(char), (shift = MAX_INPUT_SIZE - totalShifts), ptr) != shift) {
									strcat(buffer, "NOK\n");
									errFlag = 1;
									break;
								}
								fileSize -= shift;
								receiveTCPMessage(tcpSocket, buffer, MAX_INPUT_SIZE);
								totalShifts = 0;
								bufferPointer = buffer;
							}
							else {
								if (fwrite(bufferPointer, sizeof(char), fileSize, ptr) != fileSize) {
									strcat(buffer, "NOK\n");
									errFlag = 1;
									break;
								}
								bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, fileSize + 1);
								fileSize -= fileSize;
							}
						}
						if (!errFlag) strcat(strcat(reply, mid), "\n");
						fclose(ptr);
					}
				}
			}
		}
	}

	if (mode == verbose) fprintf(stdout, "PST, UID: %s, GID: %s, IP: %d, PORT: %d\n", uid, gid, addr.sin_addr.s_addr, addr.sin_port);

    sendTCPMessage(newTcpSocket, reply, strlen(reply));
}