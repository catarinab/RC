/*
 * Ficheiro: util.c
 * Autor: Luis Freire D'Andrade (N94179), Catarina da Costa Bento (N93230), Bernardo Rosa (N88077)
 * Descricao: [Projeto de RC] Development, in C language, of useful function used in the execution of commands.
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
#include "header/constants.h"

/*
 * Global Variables:
*/

int udpSocket, tcpSocket, errcode, errno;
struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
ssize_t n;
socklen_t addrlen;
struct sockaddr_in addr;

char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_IP_SIZE]; //nosso pc: localhost;
user loggedUser; 
group selectedGroup;

/*
 * Functions:
*/

/*
 * Function: verifyDigit
 * ----------------------------
 *   Checks if the string only contains digits.
 *
 *   buff: buffer where the string to check is.
 *   beg: index where the string begins.
 *   end: index where the string ends.
 *   err: error message to print.
 *
 *   returns: boolean related to errors.
 */
int verifyDigit(char buff[], int beg, int end, char err[]) {
	for (int i = beg; i < end; i ++){
		if (isdigit(buff[i]) == 0) {
			fprintf(stderr, "error: %s\n", err);
			return 1;
		}
	}
	return 0;
}

/*
 * Function: verifyAlnum
 * ----------------------------
 *   Checks if the string only contains alphanumeric characters.
 *
 *   buff: buffer where the string to check is.
 *   beg: index where the string begins.
 *   end: index where the string ends.
 *   err: error message to print.
 *
 *   returns: boolean related to errors.
 */
int verifyAlnum(char buff[], int beg, int end, char err[]) {
	for (int i = beg; i < end; i ++){
		if (isalnum(buff[i]) == 0) {
			fprintf(stderr, "error: %s\n", err);
			return 1;
		}
	}
	return 0;
}

/*
 * Function: verifyName
 * ----------------------------
 *   Checks if the string can be a file/group name.
 *
 *   buff: buffer where the string to check is.
 *   beg: index where the string begins.
 *   end: index where the string ends.
 *   err: error message to print.
 *
 *   returns: boolean related to errors.
 */
int verifyName(char buff[], int beg, int end, char err[]) {
	for (int i = beg; i < end; i ++){
		if (isalnum(buff[i]) == 0 && buff[i] != '-' && buff[i] != '_') {
			fprintf(stderr, "error: %s\n", err);
			return 1;
		}
	}
	return 0;
}

/*
 * Function: verifyName
 * ----------------------------
 *   Checks if the string only contains alphabetical characters.
 *
 *   buff: buffer where the string to check is.
 *   beg: index where the string begins.
 *   end: index where the string ends.
 *   err: error message to print.
 *
 *   returns: boolean related to errors.
 */
int verifyAlpha(char buff[], int beg, int end, char err[]) {
	for (int i = beg; i < end; i ++){
		if (isalpha(buff[i]) == 0) {
			fprintf(stderr, "error: %s \n", err);
			return 1;
		}
	}
	return 0;
}

/*
 * Function: verifyUserInfo
 * ----------------------------
 *   Checks if the information provided can be used to create a user.
 *
 *   uid: user identification.
 *   pwd: user password.
 *
 *   returns: boolean related to errors.
 */
int verifyUserInfo(char uid[], char pwd[]) {
	int errFlag = 0;

	if (strlen(uid) != 5){
		fprintf(stderr, "error: UID must have 5 numbers\n");
		errFlag = 1;
	}
	if (!errFlag) errFlag = verifyDigit(uid, 0, 5, "error: UID must contain numbers only");

	if (strlen(pwd) != 8){
		fprintf(stderr, "error: Password must have 8 characters\n");
		errFlag = 1;
	}
	if (!errFlag) errFlag = verifyAlnum(pwd, 0, 8, "Password must contain alphanumeric characters only");

	return errFlag;
}

/*
 * Function: verifyGroupInfo
 * ----------------------------
 *   Checks if the information provided can be used to create a group.
 *
 *   gid: group number.
 *   gname: group name.
 *
 *   returns: boolean related to errors.
 */
int verifyGroupInfo(char gid[], char gname[]) {
	int errFlag = 0;

	if (strlen(gid) > 2){
		fprintf(stderr, "error: GID must have no more than 2 numbers\n");
		errFlag = 1;
	}
	if (!errFlag) errFlag = verifyDigit(gid, 0, strlen(gid), "error: GID must contain numbers only");
	
	if (gname != NULL) {
		if (strlen(gname) > 24){
			fprintf(stderr, "error: Group Name must have no more than 24 alphanumeric characters\n");
			errFlag = 1;
		}
		if (!errFlag) {
			errFlag =  verifyName(gname, 0, strlen(gname), "Gname must contain alphanumeric or '-' '_' characters only");
		}
	}
	return errFlag;
}

/*
 * Function: verifySession
 * ----------------------------
 *   Checks if there is a logged user and a selected group.
 *
 *   returns: boolean related to errors.
 */
int verifySession() {
	if (!loggedUser.logged) {
		fprintf(stdout, "warning: No user logged.\n");
		return 0;
	}
	if (!selectedGroup.selected) {
		fprintf(stdout, "warning: No group selected.\n");
		return 0;
	}
	return 1;
}

/*
 * Function: resetUser
 * ----------------------------
 *   Clears the logged user's information.
 *
 */
void resetUser() {
	loggedUser.logged = 0;
	strcpy(loggedUser.uid, "");
	strcpy(loggedUser.pwd, "");
}

/*
 * Function: resetUser
 * ----------------------------
 *   Clears the selected group's information.
 *
 */
void resetGroup() {
	selectedGroup.selected = 0;
	strcpy(selectedGroup.gid, "");
}
