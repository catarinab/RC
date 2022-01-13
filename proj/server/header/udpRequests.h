/*
 * Ficheiro: udpRequests.h
 * Autor: Luis Freire D'Andrade (N94179), Catarina da Costa Bento (N93230), Bernardo Rosa (N88077)
 * Descricao: [Projeto de RC] Header file of udpRequests.c.
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

/*
 * Functions:
*/


void createUdpSocket();
void reg();
void unr();
void login();
void logout();
void gls();
void gsr();
void gur();
void glm();
void errorSendingMsg();
