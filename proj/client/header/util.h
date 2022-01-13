/*
 * Ficheiro: util.h
 * Autor: Luis Freire D'Andrade (N94179), Catarina da Costa Bento (N93230), Bernardo Rosa (N88077)
 * Descricao: [Projeto de RC] Header file of util.c.
*/

/*
 * Libraries:
*/

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

/*
 * Global Variables:
*/

extern int udpSocket, tcpSocket, errcode, errno;
extern struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
extern ssize_t n;
extern socklen_t addrlen;
extern struct sockaddr_in addr;

extern char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_IP_SIZE]; //nosso pc: localhost;
extern user loggedUser; 
extern group selectedGroup;

/*
 * Functions:
*/

int verifyDigit(char buff[], int beg, int end, char err[]);
int verifyAlnum(char buff[], int beg, int end, char err[]);
int verifyName(char buff[], int beg, int end, char err[]);
int verifyAlpha(char buff[], int beg, int end, char err[]);
int verifyUserInfo(char uid[], char pwd[]);
int verifyGroupInfo(char gid[], char gname[]);
int verifySession();
void resetUser();
void resetGroup();
