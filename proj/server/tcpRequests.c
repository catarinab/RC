#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "header/util.h"

void errorTcpSocket() {
	fprintf(stderr, "Error creating TCP Socket.\n");
	exit(1);
}

void createTcpSocket() {
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1) errorTcpSocket();

    memset(&tcpHints, 0, sizeof(tcpHints));
    tcpHints.ai_family = AF_INET;
    tcpHints.ai_socktype = SOCK_STREAM;
    tcpHints.ai_flags = AI_PASSIVE;

	errcode = getaddrinfo(NULL, port, &tcpHints, &tcpRes);
	if (errcode == -1) errorTcpSocket();

    n = bind(tcpSocket, tcpRes->ai_addr, tcpRes->ai_addrlen);
    if (n == -1) errorTcpSocket();

    if (listen(tcpSocket, 5) == -1) errorTcpSocket();
}

void sendTCPMessage(int socket, char *ptr, int nleft) {
	while (nleft > 0) {
		n = write(socket, ptr, nleft);
		if (n == -1) {
			fprintf(stderr, "Error Sending TCP Message.\n");
			exit(1);
		}
		else if (n == 0) break;
		nleft -= n;
		ptr += n;
	}
}

int receiveTCPMessage(int socket, char *ptr, int nleft) {
	int nTotal = 0;
	while (nleft > 0) {
		n = read(socket, ptr, nleft);
		if (n == -1) {
			fprintf(stderr, "Error Receiving TCP Message.\n");
			exit(1);
		}
		nTotal += n;
		if (n == 0 || ptr[n - 1] == '\n'){
			ptr[n] = '\0';
			break; 
		}
		nleft -= n;
		ptr += n;
	}
	return nTotal;
}
int movePointer(char *table, int tableSize, char **pointer, int *filled, int *totalShifts, int shift) {
	int outBounds = 0;
	if ((*totalShifts += shift) < *filled) {
		*pointer = *pointer + shift * sizeof(char);
	}
	else {
		*totalShifts -= *filled;
		outBounds = 1;
	}
	return outBounds;
}

void uls(int n) {
	DIR *d;
	FILE *ptr;
    struct dirent *dir;
	int numTokens, filled = n, outBounds, bufferSize = MAX_INPUT_SIZE - 1, shift, totalShifts = 0;
    char args[1][MAX_INFO], reply[30], pathname[20], gname[MAX_INFO], uid[MAX_INFO], *bufferPointer = buffer;

	shift = 4;
	outBounds = movePointer(buffer, bufferSize, &bufferPointer, &filled, &totalShifts, shift);
	if (outBounds) {
		memset(buffer, 0, MAX_INPUT_SIZE);
		filled = receiveTCPMessage(newTcpSocket, buffer, bufferSize);
		bufferPointer = buffer + totalShifts * sizeof(char);
	}
    numTokens = sscanf(bufferPointer, "%s\n", args[0]);
	memset(buffer, 0, MAX_INPUT_SIZE);
    strcpy(buffer, "RUL ");
    if (numTokens != 1) strcat(buffer, "NOK\n");
	else if (strcmp(args[0], "00") == 0) strcat(buffer, "NOK\n");
    else if (!(checkGroup(args[0]))) strcat(buffer, "NOK\n");
    else {
		sprintf(pathname, "GROUPS/%s/%s_name.txt", args[0], args[0]);
		memset(gname, 0, MAX_INFO);
		if (!(ptr = fopen(pathname, "r"))) {
			fprintf(stderr, "error: File open %s unsuccessful.\n", pathname);
			exit(1);
		}
		if (0 >= fread(gname, sizeof(char), MAX_INFO, ptr)) {
			fprintf(stderr, "error: File read %s unsuccessful.\n", pathname);
			exit(1);
		}
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
		else {
			fprintf(stderr, "error: Directory open %s unsuccessful.\n", pathname);
			exit(1);
		}
    }

    if (mode == verbose) fprintf(stdout, "ULS, GID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    sendTCPMessage(newTcpSocket, buffer, strlen(buffer));
}

void pst(int n) {
	FILE *ptr;
	int numTokens, filled = n, outBounds, bufferSize = MAX_INPUT_SIZE - 1, msgSize, fileSize, shift, totalShifts = 0, errFlag = 0;
    char args[2][MAX_INFO], uid[6], gid[3], mid[5] = "0000", reply[REPLY_SIZE] = "RPT ", message[MAX_MESSAGE_SIZE], pathname[45], *bufferPointer = buffer;

	shift = 4;
	outBounds = movePointer(buffer, bufferSize, &bufferPointer, &filled, &totalShifts, shift);
	if (outBounds) {
		memset(buffer, 0, MAX_INPUT_SIZE);
		filled = receiveTCPMessage(newTcpSocket, buffer, bufferSize);
		bufferPointer = buffer + totalShifts * sizeof(char);
	}
	numTokens = sscanf(bufferPointer, "%s %s ", args[0], args[1]);
	shift = strlen(args[0]) + strlen(args[1]) + 2;
	outBounds = movePointer(buffer, bufferSize, &bufferPointer, &filled, &totalShifts, shift);
	if (numTokens != 2) strcat(reply, "NOK\n");
	else if (!(checkUser(args[0]))) strcat(reply, "NOK\n");
	else if (!(checkLog(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkGroup(args[1]))) strcat(reply, "NOK\n");
	else if (!(checkSub(args[0], args[1]))) strcat(reply, "NOK\n");
	else if (!(checkMessage(args[1], mid))) strcat(reply, "NOK\n");
    else {
		if (outBounds) {
			memset(buffer, 0, MAX_INPUT_SIZE);
			filled = receiveTCPMessage(newTcpSocket, buffer, bufferSize);
			bufferPointer = buffer + totalShifts * sizeof(char);
		}
		strcpy(uid, args[0]);
		memset(args[0], 0, MAX_INFO);
		strcpy(gid, args[1]);
		memset(args[1], 0, MAX_INFO);
		numTokens = sscanf(bufferPointer, "%s ", args[0]);
		shift = strlen(args[0]) + 1;
		outBounds = movePointer(buffer, bufferSize, &bufferPointer, &filled, &totalShifts, shift);
		if (numTokens != 1) strcat(reply, "NOK\n");
		else {
			if (outBounds) {
				memset(buffer, 0, MAX_INPUT_SIZE);
				filled = receiveTCPMessage(newTcpSocket, buffer, bufferSize);
				bufferPointer = buffer + totalShifts * sizeof(char);
			}
			msgSize = atoi(args[0]);
			memset(message, 0, MAX_MESSAGE_SIZE);
			strncpy(message, bufferPointer, msgSize);
			outBounds = movePointer(buffer, bufferSize, &bufferPointer, &filled, &totalShifts, msgSize);
			if (outBounds) {
				memset(buffer, 0, MAX_INPUT_SIZE);
				filled = receiveTCPMessage(newTcpSocket, buffer, bufferSize);
				bufferPointer = buffer + totalShifts * sizeof(char);
			}
			if (!(createMsgDir(uid, gid, mid, message))) {
				fprintf(stderr, "error: Message %s directory create unsuccessful.\n", mid);
				exit(1);
			}
			memset(args[0], 0, MAX_INFO);
			memset(args[1], 0, MAX_INFO);
			numTokens = sscanf(bufferPointer, " %s %s ", args[0], args[1]);
			shift = strlen(args[0]) + strlen(args[1]) + 3;
			outBounds = movePointer(buffer, bufferSize, &bufferPointer, &filled, &totalShifts, shift);
			if (numTokens == -1 || numTokens == 0) strcat(strcat(reply, mid), "\n");
			else if (numTokens =! 2) strcat(reply, "NOK\n");
			else {
				if (outBounds) {
					memset(buffer, 0, MAX_INPUT_SIZE);
					filled = receiveTCPMessage(newTcpSocket, buffer, bufferSize);
					bufferPointer = buffer + totalShifts * sizeof(char);
				}
				fileSize = atoi(args[1]);
				sprintf(pathname, "GROUPS/%s/MSG/%s/%s", gid, mid, args[0]);
				if (!(ptr = fopen(pathname, "wb"))) {
					fprintf(stderr, "error: File open %s unsuccessful.\n", pathname);
					exit(1);
				}
				while (fileSize > 0) {
					if (fileSize >= filled - totalShifts) {
						if (fwrite(bufferPointer, sizeof(char), (shift = filled - totalShifts), ptr) != shift) {
							fprintf(stderr, "error: File read %s unsuccessful.\n", pathname);
							exit(1);
						}
						fileSize -= shift;
						memset(buffer, 0, MAX_INPUT_SIZE);
						filled = receiveTCPMessage(newTcpSocket, buffer, bufferSize);
						totalShifts = 0;
						bufferPointer = buffer;
					}
					else {
						if (fwrite(bufferPointer, sizeof(char), fileSize, ptr) != fileSize) {
							fprintf(stderr, "error: File read %s unsuccessful.\n", pathname);
							exit(1);
						}
						fileSize -= fileSize;
					}
				}
				fclose(ptr);
				strcat(strcat(reply, mid), "\n");
			}
		}
	}

	if (mode == verbose) fprintf(stdout, "PST, UID: %s, GID: %s, IP: %d, PORT: %d\n", uid, gid, addr.sin_addr.s_addr, addr.sin_port);

    sendTCPMessage(newTcpSocket, reply, strlen(reply));
}

void rtv(int n) {
	DIR *d;
    struct dirent *dir;
	FILE *ptr;
	int numTokens, filled = n, outBounds, bufferSize = MAX_INPUT_SIZE - 1, nMsg, len, fileSize, shift, totalShifts = 0, file, passedVerifications = 0;
    char args[3][MAX_INFO], message[MAX_MESSAGE_SIZE], mid[5], pathname[45], *bufferPointer = buffer;
	char fname[MAX_INFO];

	shift = 4;
	outBounds = movePointer(buffer, bufferSize, &bufferPointer, &filled, &totalShifts, shift);
	if (outBounds) {
		memset(buffer, 0, MAX_INPUT_SIZE);
		filled = receiveTCPMessage(newTcpSocket, buffer, bufferSize);
		bufferPointer = buffer + totalShifts * sizeof(char);
	}
	numTokens = sscanf(bufferPointer, "%s %s %s\n", args[0], args[1], args[2]);
	memset(buffer, 0, MAX_INPUT_SIZE);
	strcpy(buffer, "RRT ");
	bufferPointer = buffer;
	totalShifts = 0;
	shift = 4;
	movePointer(buffer, bufferSize, &bufferPointer, &bufferSize, &totalShifts, shift);
	if (numTokens != 3) strcpy(bufferPointer, "NOK\n");
	else if (!(checkUser(args[0]))) strcpy(bufferPointer, "NOK\n");
	else if (!(checkLog(args[0]))) strcpy(bufferPointer, "NOK\n");
    else if (!(checkGroup(args[1]))) strcpy(bufferPointer, "NOK\n");
	else if (!(checkSub(args[0], args[1]))) strcpy(bufferPointer, "NOK\n");
	else if (!(checkMessage(args[1], args[2]))) strcpy(bufferPointer, "EOF\n");
    else {
		passedVerifications = 1;
		nMsg = atoi(args[2]) + 19;
        if (nMsg < 100) sprintf(pathname, "GROUPS/%s/MSG/00%d", args[1], nMsg);
        else if (nMsg < 1000) sprintf(pathname, "GROUPS/%s/MSG/0%d", args[1], nMsg);
        else sprintf(pathname, "GROUPS/%s/MSG/%d", args[1], nMsg);
    	if (access(pathname, F_OK) == 0) nMsg = 20;
		else nMsg = countMessages(args[1], nMsg - 19) + 1;
		sprintf(mid, "%d", nMsg);
		sprintf(bufferPointer, "OK %d", nMsg);
		shift = 2 + strlen(mid) + 1;
		movePointer(buffer, bufferSize, &bufferPointer, &bufferSize, &totalShifts, shift);
		for (int i = atoi(args[2]); i < atoi(args[2]) + nMsg; i++) {
			//MID
			if (i < 10) sprintf(mid, "000%d", i);
			else if (i < 100) sprintf(mid, "00%d", i);
        	else if (nMsg < 1000) sprintf(mid, "0%d", i);
        	else sprintf(mid, "%d", i);
			sprintf(bufferPointer, " %s ", mid);
			shift = strlen(mid) + 2;
			movePointer(buffer, bufferSize, &bufferPointer, &bufferSize, &totalShifts, shift);
			//UID
			sprintf(bufferPointer, "%s ", args[0]);
			shift = strlen(args[0]) + 1;
			movePointer(buffer, bufferSize, &bufferPointer, &bufferSize, &totalShifts, shift);
			//TEXTSIZE
			sprintf(pathname, "GROUPS/%s/MSG/%s/T E X T.txt", args[1], mid);
			if (!(ptr = fopen(pathname, "rb"))) {
				fprintf(stderr, "error: File open %s unsuccessful.\n", pathname);
				exit(1);
			}
			fseek(ptr, 0L, SEEK_END);
			len = ftell(ptr);
			fseek(ptr, 0L, SEEK_SET);
			sprintf(bufferPointer, "%d ", len);
			shift = strlen(bufferPointer);
			movePointer(buffer, bufferSize, &bufferPointer, &bufferSize, &totalShifts, shift);
			//MESSAGE
			if (len != fread(bufferPointer, sizeof(char), bufferSize, ptr)) {
				fprintf(stderr, "error: File read %s unsuccessful.\n", pathname);
				exit(1);
			}
			fclose(ptr);
			shift = len;
			movePointer(buffer, bufferSize, &bufferPointer, &bufferSize, &totalShifts, shift);
			//FILE
			file = 0;
			memset(pathname, 0, 45);
			sprintf(pathname, "GROUPS/%s/MSG/%s", args[1], mid);
			d = opendir(pathname);
			if (d) {
				while ((dir = readdir(d)) != NULL) {
					if (dir->d_name[0] == '.')
						continue;
					else if (strcmp(dir->d_name, "T E X T.txt") == 0)
						continue;
					else if (strcmp(dir->d_name, "A U T H O R.txt") == 0)
						continue;
					else {
						file = 1;
						memset(fname, 0, MAX_INFO);
						sprintf(fname, "%s", dir->d_name);
						break;
					}
				}
				closedir(d);
			}
			else {
				fprintf(stderr, "error: Directory open %s unsuccessful.\n", pathname);
				exit(1);
			}
			if (file) {
				sprintf(pathname, "GROUPS/%s/MSG/%s/%s", args[1], mid, fname);
				if (!(ptr = fopen(pathname, "rb"))) {
					fprintf(stderr, "error: File open %s unsuccessful.\n", pathname);
					exit(1);
				}
				fseek(ptr, 0L, SEEK_END);
				len = ftell(ptr);
				fseek(ptr, 0L, SEEK_SET);
				sprintf(bufferPointer, " / %s %d ", fname, len);
				shift = strlen(bufferPointer);
				movePointer(buffer, bufferSize, &bufferPointer, &bufferSize, &totalShifts, shift);
				while (0 < (len = fread(bufferPointer, sizeof(char), bufferSize - totalShifts, ptr))) {
					sendTCPMessage(newTcpSocket, buffer, totalShifts + len);
					memset(buffer, 0, MAX_INPUT_SIZE);
					bufferPointer = buffer;
					totalShifts = 0;
				}
				fclose(ptr);
			}
			else {
				sendTCPMessage(newTcpSocket, buffer, totalShifts);
				memset(buffer, 0, MAX_INPUT_SIZE);
				bufferPointer = buffer;
				totalShifts = 0;
			}
		}
		sendTCPMessage(newTcpSocket, "\n", 1);
	}

	if (mode == verbose) fprintf(stdout, "RTV, UID: %s, GID: %s, IP: %d, PORT: %d\n", args[0], args[1], addr.sin_addr.s_addr, addr.sin_port);
	
	if (!passedVerifications) sendTCPMessage(newTcpSocket, buffer, strlen(buffer));
}