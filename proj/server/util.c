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

int delUserDir(char *uid) {
    char pathname[30];

    sprintf(pathname,"USERS/%s/%s_login.txt", uid, uid);
    if (access(pathname, F_OK) == 0)
        if (unlink(pathname) != 0) return 0;

    sprintf(pathname,"USERS/%s/%s_pass.txt", uid, uid);
    if (unlink(pathname) != 0) return 0;

    sprintf(pathname,"USERS/%s",uid);
    if (rmdir(pathname) != 0) return 0;

    return 1;
}

int checkPass(char *uid, char *pass) {
    FILE * ptr;
    char pathname[30], filePass[9];

    sprintf(pathname, "USERS/%s/%s_pass.txt", uid, uid);
    if (!(ptr = fopen(pathname, "r"))) return 0;
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