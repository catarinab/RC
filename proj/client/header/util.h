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

extern int udpSocket, tcpSocket, errcode, errno;
extern struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
extern ssize_t n;
extern socklen_t addrlen;
extern struct sockaddr_in addr;

extern char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_IP_SIZE]; //nosso pc: localhost;
extern user loggedUser; 
extern group selectedGroup;

int verifyDigit(char buff[], int beg, int end, char err[]);
int verifyAlnum(char buff[], int beg, int end, char err[]);
int verifyAlpha(char buff[], int beg, int end, char err[]);
int verifyUserInfo(char uid[], char pwd[]);
int verifySession();
void resetUser();
void resetGroup();
int verifyGroupInfo(char gid[], int flag, char gname[]);
