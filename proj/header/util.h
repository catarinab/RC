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

void concatenateArgs(char *final, char args[][MAX_INFO], int argsNumber);
int verifyUserInfo(char uid[], char pwd[]);

void resetUser();
void resetGroup();