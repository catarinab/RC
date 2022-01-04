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
		else if (n == 0) break;
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
    else if (checkGroup(args[0])) strcat(buffer, "NOK\n");
    else {
		sprintf(pathname, "GROUPS/%s/%s_name.txt", args[0], args[0]);
		memset(gname, 0, MAX_INFO);
		if (!(ptr = fopen(pathname, "r"))) exit(1);
		if (0 >= fread(gname, sizeof(char), MAX_INFO, ptr)) exit(1);
		fclose(ptr);
		sprintf(reply, "OK %s", gname);
		strcat(buffer, reply);
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