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
    int nDir, nMsg = 0;
    char pathname[20], gid[3], gname[MAX_INFO], mid[5], reply[33];
    
    nDir = countGroups();
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
            nMsg = countMessages(gid);
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

void gsr() {
    int numTokens, nDir;
    char args[3][MAX_INFO], gid[3], reply[REPLY_SIZE] = "RGS ";

    numTokens = sscanf(buffer, "%s %s %s", args[0], args[1], args[2]);
    if (numTokens != 3) strcat(reply, "NOK\n");
    else if (!(checkUser(args[0]))) strcat(reply, "E_USR\n");
    else if (!(checkLog(args[0]))) strcat(reply, "NOK\n");
    else if (checkGroup(args[1]) == 0) strcat(reply, "E_GRP\n");
    else if (!(checkGroupInfo(args[1], args[2]))) strcat(reply, "E_GNAME\n");
    else if (checkGroup(args[1]) == 99) strcat(reply, "E_FULL\n");
    else {
        if (strcmp(args[1], "00") == 0) {
            nDir = countGroups();
            if (nDir < 10) sprintf(gid, "0%d", nDir + 1);
            else sprintf(gid, "%d", nDir + 1);
            if (!(createGroupDir(gid, args[2]))) strcat(reply, "NOK\n");
            strcpy(args[1], gid);
        }
        if (!(createSubFile(args[0], args[1]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "GSR, UID: %s, GID: %s, IP: %d, PORT: %d\n", args[0], args[1], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

void gur() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "RGU ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(checkUser(args[0]))) strcat(reply, "E_USR\n");
    else if (!(checkLog(args[0]))) strcat(reply, "NOK\n");
    else if (checkGroup(args[1]) == 0) strcat(reply, "E_GRP\n");
    else {
        if (!(deleteSubFile(args[0], args[1]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "GUR, UID: %s, GID: %s, IP: %d, PORT: %d\n", args[0], args[1], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);   
}

void glm() {
    FILE * ptr;
    int numTokens, nGroups, nSubTo = 0, nMsg;
    char args[1][MAX_INFO], pathname[20], gid[3], gname[MAX_INFO], mid[5], reply[33];

    numTokens = sscanf(buffer, "%s", args[0]);
    if (numTokens != 1) strcat(reply, "NOK\n");
    else if (!(checkUser(args[0]))) strcat(reply, "E_USR\n");
    else if (!(checkLog(args[0]))) strcat(reply, "E_USR\n");
    else {
        nGroups = countGroups();
        memset(buffer, 0, MAX_INPUT_SIZE);
        if (nGroups > 0) {
            for (int i = 1; i <= nGroups; i++) {
                if (i < 10) sprintf(gid, "0%d", i);
                else sprintf(gid, "%d", i);
                sprintf(pathname, "GROUPS/%s/%s.txt", gid, args[0]);
                if (access(pathname, F_OK) != 0) continue;
                nSubTo++;
                sprintf(pathname, "GROUPS/%s/%s_name.txt", gid, gid);
                memset(gname, 0, MAX_INFO);
                if (!(ptr = fopen(pathname, "r"))) exit(1);
                if (0 >= fread(gname, sizeof(char), MAX_INFO, ptr)) exit(1);
                fclose(ptr);
                nMsg = countMessages(gid);
                if (nMsg < 10) sprintf(mid, "000%d", nMsg);
                else if (nMsg < 100) sprintf(mid, "00%d", nMsg);
                else if (nMsg < 1000) sprintf(mid, "0%d", nMsg);
                else sprintf(mid, "%d", nMsg);
                sprintf(reply, " %s %s %s", gid, gname, mid);
                strcat(buffer, reply);
            }
        }
        sprintf(buffer, "RGM %d%s\n", nSubTo, buffer);
    }

    if (mode == verbose) fprintf(stdout, "GUR, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, buffer, strlen(buffer), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}
