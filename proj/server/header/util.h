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

extern int udpSocket, tcpSocket, newTcpSocket, errcode, errno;
extern struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
extern struct sockaddr_in addr;
extern ssize_t n;
extern socklen_t addrlen;
extern char userIP[INET_ADDRSTRLEN];

extern char buffer[MAX_INPUT_SIZE], port[6];

extern enum {verbose, quiet} mode;

/*
 * Functions:
*/

int verifyUserInfo(char uid[], char pwd[]);
int createUserDir(char *uid, char *pass);
int checkUserExists(char *uid);
int delUserDir(char *uid);
int checkPass(char *uid, char *pass);
int createLogFile(char *uid);
int checkLog(char *uid);
int deleteLogFile(char *uid);
int checkUser(char *uid);
int checkGroup(char *gid);
int checkGroupInfo(char *gid, char *gName);
int countGroups();
int createGroupDir(char *gid, char *gname);
int createSubFile(char *uid, char *gid);
int deleteSubFile(char *uid, char *gid);
int countMessages(char *gid, int mid);
int verifyUserFile(char * userFile, char * uid);
int checkMessage(char *gid, char *mid);
int createMsgDir(char *uid, char *gid, char *mid, char *text);
int checkSub(char * uid, char * gid);