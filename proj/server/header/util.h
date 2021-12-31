#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include "constants.h"

extern int udpSocket, tcpSocket, newTcpSocket, errcode, errno;
extern struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
extern struct sockaddr_in addr;
extern ssize_t n;
extern socklen_t addrlen;

extern char buffer[MAX_INPUT_SIZE], port[6];

extern enum {verbose, quiet} mode;


int verifyUserInfo(char uid[], char pwd[]);
int createUserDir(char *uid, char *pass);
int checkUserExists(char *uid);
int delUserDir(char *uid);
int checkPass(char *uid, char *pass);
int createLogFile(char *uid);
int checkLog(char *uid);
int deleteLogFile(char *uid);