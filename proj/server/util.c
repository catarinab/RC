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
#include "header/constants.h"

int udpSocket, tcpSocket, newTcpSocket, errcode, errno;
struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
struct sockaddr_in addr;
ssize_t n;
socklen_t addrlen;

char buffer[MAX_INPUT_SIZE], port[6];

enum {verbose, quiet} mode;


int verifyDigit(char buff[], int beg, int end) {
	for (int i = beg; i < end; i ++){
		if (isdigit(buff[i]) == 0) return 0;
    }
	return 1;
}

int verifyAlnum(char buff[], int beg, int end) {
	for (int i = beg; i < end; i ++){
		if (isalnum(buff[i]) == 0) return 0;
	}
	return 1;
}

int verifyUserInfo(char uid[], char pwd[]) {
	if (strlen(uid) != 5 || strlen(pwd) != 8 || !verifyDigit(uid, 0, strlen(uid)) || !verifyAlnum(pwd, 0, strlen(pwd))) 
        return 0;
	return 1;
}

int createUserDir(char *uid, char *pass) {
    int size;
    FILE * ptr;
    char pathname[30];

    sprintf(pathname,"USERS/%s",uid);
    if ((mkdir(pathname, 0700)) == -1) return 0;

    sprintf(pathname, "USERS/%s/%s_pass.txt", uid, uid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    if (fwrite(pass, sizeof(char), (size = strlen(pass)), ptr) != size) return 0;
    fclose(ptr);

    return 1;
}

int checkUserExists(char *uid) {
    char pathname[25];

    sprintf(pathname, "USERS/%s", uid);
    if (access(pathname, F_OK) == 0) return 1;
    else return 0;
}

int countGroups() {
    int nDir = 0;
    char pathname[20];
    
    strcpy(pathname, "GROUPS/01");
    while (access(pathname, F_OK) == 0) {
        nDir++;
        if (nDir < 9) sprintf(pathname, "GROUPS/0%d", (nDir + 1));
        else sprintf(pathname, "GROUPS/%d", (nDir + 1));
    }
    return nDir;
}

int countMessages(char *gid) {
    int nMsg = 0;
    char pathname[20];

    sprintf(pathname, "GROUPS/%s/MSG/0001", gid);
    while (access(pathname, F_OK) == 0) {
        nMsg++;
        if (nMsg < 9) sprintf(pathname, "GROUPS/000%d", (nMsg + 1));
        else if (nMsg < 99) sprintf(pathname, "GROUPS/00%d", (nMsg + 1));
        else if (nMsg < 999) sprintf(pathname, "GROUPS/0%d", (nMsg + 1));
        else sprintf(pathname, "GROUPS/%d", (nMsg + 1));
    }
    return nMsg;
}

int delUserDir(char *uid) {
    int nGroups;
    char pathname[30], gid[3];

    sprintf(pathname,"USERS/%s/%s_login.txt", uid, uid);
    if (access(pathname, F_OK) == 0)
        if (unlink(pathname) != 0) return 0;

    sprintf(pathname,"USERS/%s/%s_pass.txt", uid, uid);
    if (unlink(pathname) != 0) return 0;

    nGroups = countGroups();
    if (nGroups > 0) {
        for (int i = 1; i <= nGroups; i++) {
            if (i < 10) sprintf(gid, "0%d", i);
            else sprintf(gid, "%d", i);
            sprintf(pathname, "GROUPS/%s/%s.txt", gid, uid);
            if (access(pathname, F_OK) == 0)
                if (unlink(pathname) != 0) return 0;
        }
    }

    sprintf(pathname,"USERS/%s",uid);
    if (rmdir(pathname) != 0) return 0;

    return 1;
}

int checkPass(char *uid, char *pass) {
    FILE * ptr;
    char pathname[30], filePass[9];

    sprintf(pathname, "USERS/%s/%s_pass.txt", uid, uid);
    if (!(ptr = fopen(pathname, "r"))) return 0;
    memset(filePass, 0, 9);
    if (8 != fread(filePass, sizeof(char), 8, ptr)) return 0;
    if (strcmp(filePass, pass) != 0) return 0;
    fclose(ptr);

    return 1;
}

int createLogFile(char *uid) {
    FILE * ptr;
    char pathname[30];

    sprintf(pathname, "USERS/%s/%s_login.txt", uid, uid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    fclose(ptr);

    return 1;
}

int checkLog(char *uid) {
    char pathname[30];

    sprintf(pathname, "USERS/%s/%s_login.txt", uid, uid);
    if (access(pathname, F_OK) == 0) return 1;
    else return 0;
}

int deleteLogFile(char *uid) {
    char pathname[30];

    sprintf(pathname,"USERS/%s/%s_login.txt", uid, uid);
    if (unlink(pathname) != 0) return 0;

    return 1;
}

int checkUser(char *uid) {
    char pathname[15];

    sprintf(pathname, "USERS/%s", uid);
    if (access(pathname, F_OK) == 0) return 1;
    else return 0;
}

int checkGroup(char *gid) {
    char pathname[10];

    if (strcmp(gid, "00") == 0) {
        if (access("GROUPS/99", F_OK) == 0) return 99;
        else return 1;
    }
    else {
        sprintf(pathname, "GROUPS/%s", gid);
        if (access(pathname, F_OK) == 0) return 1;
        else return 0;
    }
}

int checkGroupInfo(char *gid, char *gname) {
    FILE * ptr;
    int nDir = 0;
    char pathname[25], fileName[25];
    
    if (strcmp(gid, "00") == 0) return 1;

    sprintf(pathname, "GROUPS/%s/%s_name.txt", gid, gid);
    if (!(ptr = fopen(pathname, "r"))) return 0;
    memset(fileName, 0, 25);
    if (0 >= fread(fileName, sizeof(char), 24, ptr)) return 0;
    if (strcmp(fileName, gname) != 0) return 0;
    fclose(ptr);

    return 1;
}

int createGroupDir(char *gid, char *gname) {
    int size;
    FILE * ptr;
    char pathname[25];

    sprintf(pathname, "GROUPS/%s", gid);
    if ((mkdir(pathname, 0700)) == -1) return 0;
    sprintf(pathname, "GROUPS/%s/MSG", gid);
    if ((mkdir(pathname, 0700)) == -1) return 0;

    sprintf(pathname, "GROUPS/%s/%s_name.txt", gid, gid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    if (fwrite(gname, sizeof(char), (size = strlen(gname)), ptr) != size) return 0;
    fclose(ptr);

    return 1;
}

int createSubFile(char *uid, char *gid) {
    FILE * ptr;
    char pathname[20];

    sprintf(pathname, "GROUPS/%s/%s.txt", gid, uid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    fclose(ptr);

    return 1;
}

int deleteSubFile(char *uid, char *gid) {
    char pathname[20];

    sprintf(pathname,"GROUPS/%s/%s.txt", gid, uid);
    if (unlink(pathname) != 0) return 0;

    return 1;
}

int verifyUserFile(char * userFile, char * uid) {
    char ext[4];
    int numTokens;

    memset(uid, 0, MAX_INFO);
    numTokens = scanf(userFile, "%s.%s", uid, ext);
    if(verifyDigit(uid, 0, strlen(uid)) && strlen(uid) == 5 && strcmp(ext, "txt") == 0) return 1;
    else return 0;
}