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

char * movePointer(char *table, int tableSize , char *pointer, int *totalShifts, int shift) {
	if ((*totalShifts += shift) < tableSize) {
		pointer = pointer + shift * sizeof(char);
	}
	else {
		*totalShifts -= tableSize;
		receiveTCPMessage(newTcpSocket, table, tableSize);
		pointer = table + (*totalShifts) * sizeof(char);
	}
	return pointer;
}

void uls() {
	DIR *d;
	FILE *ptr;
    struct dirent *dir;
	int numTokens, shift, totalShifts = 0;
    char args[1][MAX_INFO], reply[30], pathname[20], gname[MAX_INFO], uid[MAX_INFO], *bufferPointer = buffer;

	shift = 4;
	bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
    numTokens = sscanf(bufferPointer, "%s\n", args[0]);
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

void pst() {
	FILE *ptr;
	int numTokens, nMsgize, fileSize, shift, totalShifts = 0, errFlag = 0;
    char args[2][MAX_INFO], uid[6], gid[3], mid[5] = "0000", reply[REPLY_SIZE] = "RPT ", message[MAX_MESSAGE_SIZE], pathname[45], *bufferPointer = buffer;

	shift = 4;
	bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
	numTokens = sscanf(bufferPointer, "%s %s ", args[0], args[1]);
	shift = strlen(args[0]) + strlen(args[1]) + 2;
	bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
	if (numTokens != 2) strcat(reply, "NOK\n");
	else if (!(checkUser(args[0]))) strcat(reply, "NOK\n");
	else if (!(checkLog(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkGroup(args[1]))) strcat(reply, "NOK\n");
	else if (!(checkSub(args[0], args[1]))) strcat(reply, "NOK\n");
	else if (!(checkMessage(args[1], mid))) strcat(reply, "NOK\n");
    else {
		strcpy(uid, args[0]);
		memset(args[0], 0, MAX_INFO);
		strcpy(gid, args[1]);
		memset(args[1], 0, MAX_INFO);
		numTokens = sscanf(bufferPointer, "%s ", args[0]);
		shift = strlen(args[0]) + 1;
		bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
		if (numTokens != 1) strcat(reply, "NOK\n");
		else {
			nMsgize = atoi(args[0]);
			memset(message, 0, MAX_MESSAGE_SIZE);
			strncpy(message, bufferPointer, nMsgize);
			bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, nMsgize);
			if (!(createMsgDir(uid, gid, mid, message))) strcat(reply, "NOK\n");
			else {
				memset(args[0], 0, MAX_INFO);
				memset(args[1], 0, MAX_INFO);
				numTokens = sscanf(bufferPointer, " %s %s ", args[0], args[1]);
				shift = strlen(args[0]) + strlen(args[1]) + 3;
				bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
				if (numTokens == -1 || numTokens == 0) strcat(strcat(reply, mid), "\n");
				else if (numTokens =! 2) strcat(reply, "NOK\n");
				else {
					fileSize = atoi(args[1]);
					sprintf(pathname, "GROUPS/%s/MSG/%s/%s", gid, mid, args[0]);
					if (!(ptr = fopen(pathname, "wb"))) strcat(reply, "NOK\n");
					else {
						while (fileSize > 0) {
							if (fileSize >= MAX_INPUT_SIZE - totalShifts) {
								if (fwrite(bufferPointer, sizeof(char), (shift = MAX_INPUT_SIZE - totalShifts), ptr) != shift) {
									strcat(buffer, "NOK\n");
									errFlag = 1;
									break;
								}
								fileSize -= shift;
								memset(buffer, 0, MAX_INPUT_SIZE);
								receiveTCPMessage(newTcpSocket, buffer, MAX_INPUT_SIZE);
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

void rtv() {
	FILE *ptr;
	int numTokens, nMsg, nMsgize, fileSize, shift, totalShifts = 0;
    char args[3][MAX_INFO], message[MAX_MESSAGE_SIZE], mid[5], pathname[45], *bufferPointer = buffer;

	shift = 4;
	bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
	numTokens = sscanf(bufferPointer, "%s %s %s\n", args[0], args[1], args[2]);
	shift = strlen(args[0]) + strlen(args[1]) + strlen(args[2]) + 2;
	bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
	memset(buffer, 0, MAX_INPUT_SIZE);
	strcpy(buffer, "RRT ");
	bufferPointer = buffer;
	totalShifts = 0;
	shift = 4;
	bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
	if (numTokens != 3) strcpy(bufferPointer, "NOK\n");
	else if (!(checkUser(args[0]))) strcpy(bufferPointer, "NOK\n");
	else if (!(checkLog(args[0]))) strcpy(bufferPointer, "NOK\n");
    else if (!(checkGroup(args[1]))) strcpy(bufferPointer, "NOK\n");
	else if (!(checkSub(args[0], args[1]))) strcpy(bufferPointer, "NOK\n");
	else if (!(checkMessage(args[1], args[2]))) strcpy(bufferPointer, "EOF\n");
    else {
		nMsg = atoi(args[2]) + 19;
        if (nMsg < 100) sprintf(pathname, "GROUPS/%s/MSG/00%d", args[1], nMsg);
        else if (nMsg < 1000) sprintf(pathname, "GROUPS/%s/MSG/0%d", args[1], nMsg);
        else sprintf(pathname, "GROUPS/%s/MSG/%d", nMsg);
    	if (access(pathname, F_OK) == 0) nMsg = 20;
		else nMsg = countMessages(args[1], nMsg - 19) + 1;
		sprintf(mid, "%d", nMsg);
		sprintf(bufferPointer, "OK %d ", nMsg);
		shift = 2 + strlen(mid) + 2;
		bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
		for (int i = atoi(args[2]); i < atoi(args[2]) + nMsg; i++) {
			//MID
			if (5 >= MAX_INPUT_SIZE - totalShifts) {
				sendTCPMessage(newTcpSocket, buffer, strlen(buffer));
				memset(buffer, 0, MAX_INPUT_SIZE);
				bufferPointer = buffer;
				totalShifts = 0;
			}
			if (i < 10) sprintf(mid, "000%d", i);
			else if (i < 100) sprintf(mid, "00%d", i);
        	else if (nMsg < 1000) sprintf(mid, "0%d", i);
        	else sprintf(mid, "%d", i);
			sprintf(bufferPointer, "%s ", mid);
			shift = strlen(mid) + 1;
			bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
			//UID
			if (6 >= MAX_INPUT_SIZE - totalShifts) {
				sendTCPMessage(newTcpSocket, buffer, strlen(buffer));
				memset(buffer, 0, MAX_INPUT_SIZE);
				bufferPointer = buffer;
				totalShifts = 0;
			}
			sprintf(bufferPointer, "%s ", args[0]);
			shift = strlen(args[0]) + 1;
			bufferPointer = movePointer(buffer, MAX_INPUT_SIZE, bufferPointer, &totalShifts, shift);
			//TEXTSIZE
		}
	}

	if (mode == verbose) fprintf(stdout, "RTV, UID: %s, GID: %s, IP: %d, PORT: %d\n", args[0], args[1], addr.sin_addr.s_addr, addr.sin_port);

    sendTCPMessage(newTcpSocket, buffer, strlen(buffer));
}