#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include "header/util.h"

void createUdpSocket() {
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1) exit(1);

	memset(&udpHints, 0, sizeof(udpHints));
	udpHints.ai_family = AF_INET;
	udpHints.ai_socktype = SOCK_DGRAM;
    udpHints.ai_flags = AI_PASSIVE;
    
	errcode = getaddrinfo(NULL, port, &udpHints, &udpRes);
	if (errcode == -1) exit(1);
    
    n = bind(udpSocket, udpRes->ai_addr, udpRes->ai_addrlen);
    if (n == 1) exit(1);
}

void reg() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "RRG ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(verifyUserInfo(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (checkUserExists(args[0])) strcat(reply, "DUP\n");
    else {
        if (!(createUserDir(args[0], args[1]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "REG, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

void unr() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "RUN ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(verifyUserInfo(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (!(checkUserExists(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkPass(args[0], args[1]))) strcat(reply, "NOK\n");
    else {
        if (!(delUserDir(args[0]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "UNR, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

void login() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "RLO ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(verifyUserInfo(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (!(checkUserExists(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkPass(args[0], args[1]))) strcat(reply, "NOK\n");
    else {
        if (!(createLogFile(args[0]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "LOG, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

void logout() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "ROU ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(verifyUserInfo(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (!(checkUserExists(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkPass(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (!(checkLog(args[0]))) strcat(reply, "NOK\n");
    else {
        if (!(deleteLogFile(args[0]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "OUT, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

void gls() {
    FILE * ptr;
    int nDir = 0, nMsg = 0;
    char pathname[20], gid[3], gname[MAX_INFO], mid[5], reply[33];
    
    strcpy(pathname, "GROUPS/01");
    while (access(pathname, F_OK) == 0) {
        nDir++;
        if (nDir < 9) sprintf(pathname, "GROUPS/0%d", (nDir + 1));
        else sprintf(pathname, "GROUPS/%d", (nDir + 1));
    }

    memset(buffer, 0, MAX_INPUT_SIZE);
    sprintf(buffer, "RGL %d", nDir);
    if (nDir > 0) {
        for (int i = 1; i <= nDir; i++) {
            if (i < 10) sprintf(gid, "0%d", i);
            else sprintf(gid, "%d", i);
            sprintf(pathname, "GROUPS/%s/%s_name.txt", gid, gid);
            memset(gname, 0, MAX_INFO);
            if (!(ptr = fopen(pathname, "r"))) exit(1);
            if (0 >= fread(gname, sizeof(char), MAX_INFO, ptr)) exit(1);
            fclose(ptr);
            sprintf(pathname, "GROUPS/%s/MSG/0001", gid, gid);
            while (access(pathname, F_OK) == 0) {
                nMsg++;
                if (nMsg < 9) sprintf(pathname, "GROUPS/000%d", (nMsg + 1));
                else if (nMsg < 99) sprintf(pathname, "GROUPS/00%d", (nMsg + 1));
                else if (nMsg < 999) sprintf(pathname, "GROUPS/0%d", (nMsg + 1));
                else sprintf(pathname, "GROUPS/%d", (nMsg + 1));
            }
            if (nMsg < 10) sprintf(mid, "000%d", nMsg);
            else if (nMsg < 100) sprintf(mid, "00%d", nMsg);
            else if (nMsg < 1000) sprintf(mid, "0%d", nMsg);
            else sprintf(mid, "%d", nMsg);
            sprintf(reply, " %s %s %s", gid, gname, mid);
            strcat(buffer, reply);
        }
    }
    strcat(buffer, "\n");

    if (mode == verbose) fprintf(stdout, "GLS, IP: %d, PORT: %d\n", addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, buffer, strlen(buffer), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}
