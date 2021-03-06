/*
 * Ficheiro: udpRequests.c
 * Autor: Luis Freire D'Andrade (N94179), Catarina da Costa Bento (N93230), Bernardo Rosa (N88077)
 * Descricao: [Projeto de RC] Development, in C language, of the user commands that receive information from the server by UDP.
*/

/*
 * Libraries:
*/

#include "header/util.h"
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

/*
 * Function: createUdpSocket
 * ----------------------------
 *   Creates the UDP file descriptor.
 *
 */
void createUdpSocket() {
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1) exit(1);

	memset(&udpHints, 0, sizeof(udpHints));
	udpHints.ai_family = AF_INET;
	udpHints.ai_socktype = SOCK_DGRAM;

	errcode = getaddrinfo(ip, port, &udpHints, &udpRes);
	if (errcode == -1) exit(1);
}

/*
 * Function: timerOn
 * ----------------------------
 *   Sets a timer in the UDP file descriptor.
 *
 */
int timerON() {
	struct timeval tmout;
	memset((char *)&tmout, 0, sizeof(tmout)); /* clear time structure */
	tmout.tv_sec = 15; /* Wait for 15 sec for a reply from server. */
	return(setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout, sizeof(struct timeval)));
}

/*
 * Function: timerOff
 * ----------------------------
 *   Resets the timer in the UDP file descriptor.
 *
 */
int timerOFF() {
	struct timeval tmout;
	memset((char *)&tmout, 0, sizeof(tmout)); /* clear time structure */
	return(setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout, sizeof(struct timeval)));
}

/*
 * Function: receiveUDPMessage
 * ----------------------------
 *   Receives information from the server in the UDP file descriptor. 
 *
 */
void receiveUDPMessage() {
	addrlen = sizeof(addr);
	if(timerON() < 0) {
		fprintf(stderr, "error: UDP File Descriptor problem.\n");
		exit(1);
	}
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n < 0) {
		fprintf(stderr, "error: UDP File Descriptor problem.\n");
		exit(1);
	}
	timerOFF();
}

/*
 * Function: reg
 * ----------------------------
 *   Executes the register command.
 *
 */
void reg() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "REG ", argsCommand[MAX_COMMAND_SIZE] = "";

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);

	if (numTokens != 2) {
		fprintf(stderr, "error: incorrect command arguments\n");
		return;
	}
	if (verifyUserInfo(args[0], args[1])) return;

	sprintf(argsCommand, "%s %s\n", args[0], args[1]);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	receiveUDPMessage();

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);
	if (numTokens != 2 || strcmp(args[0], "RRG") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "OK") == 0) fprintf(stdout, "User successfully registered.\n");
	else if (strcmp(args[1], "DUP") == 0) fprintf(stdout, "User already registered.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "User not registered.\n");
	else exit(1);
}

/*
 * Function: unr
 * ----------------------------
 *   Executes the unregister command.
 *
 */
void unr() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "UNR ", argsCommand[MAX_COMMAND_SIZE] = "";

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);

	if (numTokens != 2) {
		fprintf(stderr, "error: incorrect command arguments\n");
		return;
	}
	if (verifyUserInfo(args[0], args[1])) return;

	sprintf(argsCommand, "%s %s\n", args[0], args[1]);
	strcat(command, argsCommand);

	if(strcmp(loggedUser.uid, args[0]) == 0)
		resetUser();

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	receiveUDPMessage();

	numTokens = sscanf(buffer, "  %s %s", args[0], args[1]);
	if (numTokens != 2 || strcmp(args[0], "RUN") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "OK") == 0) fprintf(stdout, "User successfully unregistered.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "User not unregistered.\n");
	else exit(1);
}

/*
 * Function: login
 * ----------------------------
 *   Executes the login command.
 *
 */
void login() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "LOG ", argsCommand[MAX_COMMAND_SIZE] = "";

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);

	if (loggedUser.logged) {
		fprintf(stderr, "error: User %s logged in, please logout first.\n", loggedUser.uid);
		return;
	}

	if (numTokens != 2) {
		fprintf(stderr, "error: incorrect command arguments\n");
		return;
	}
	if (verifyUserInfo(args[0], args[1])) return;

	sprintf(argsCommand, "%s %s\n", args[0], args[1]);
	strcat(command, argsCommand);
	strcpy(loggedUser.uid, args[0]);
	strcpy(loggedUser.pwd, args[1]);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	receiveUDPMessage();

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);
	if (numTokens != 2 || strcmp(args[0], "RLO") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "OK") == 0) {
		fprintf(stdout, "User successfully logged in.\n");
		loggedUser.logged = 1;
	}
	else if (strcmp(args[1], "NOK") == 0) {
		fprintf(stdout, "User not logged in.\n");
		resetUser();
	}
	else exit(1);
}

/*
 * Function: logout
 * ----------------------------
 *   Executes the logout command.
 *
 */
void logout() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "OUT ", argsCommand[MAX_COMMAND_SIZE] = "";

	if (!loggedUser.logged) {
		fprintf(stderr, "error: No logged user.\n");
		return;
	}
	
	sprintf(argsCommand, "%s %s\n", loggedUser.uid, loggedUser.pwd);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	receiveUDPMessage();

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);
	if (numTokens != 2 || strcmp(args[0], "ROU") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "OK") == 0) {
		fprintf(stdout, "User successfully logged out.\n");
		resetUser();
	}
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "User not logged out.\n");
	else exit(1);
}

/*
 * Function: su
 * ----------------------------
 *   Executes the show UID command.
 *
 */
void su() {
	if (loggedUser.logged) fprintf(stdout, "UID of logged user: %s.\n", loggedUser.uid);
	else fprintf(stdout, "No logged user.\n");
}

/*
 * Function: gl
 * ----------------------------
 *   Executes the group list command.
 *
 */
void gl() {
	int numTokens;
	char args[3][MAX_INFO], command[MAX_COMMAND_SIZE] = "GLS\n", argsCommand[MAX_COMMAND_SIZE] = "";

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	receiveUDPMessage();

	numTokens = sscanf(buffer, " %s %s %[^\n]", args[0], args[1], buffer);
	if (numTokens < 2 || strcmp(args[0], "RGL") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "0") == 0) fprintf(stdout, "No groups available.\n");
	else {
		int n = atoi(args[1]);
		for (int i = 0; i < n; i++) {
			numTokens = sscanf(buffer, " %s %s %s %[^\n]", args[0], args[1], args[2], buffer);
			fprintf(stdout, "Group ID: %s, Group Name: %s, Last Message: %s\n", args[0], args[1], args[2]);
		}
	}
}

/*
 * Function: sub
 * ----------------------------
 *   Executes the subscribe command.
 *
 */
void sub() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "GSR ", argsCommand[MAX_COMMAND_SIZE] = "";

	if (!loggedUser.logged) {
		fprintf(stderr, "error: No logged user.\n");
		return;
	}

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);

	if (numTokens != 2) {
		fprintf(stderr, "error: Incorrect command arguments\n");
		return;
	}
	if (verifyGroupInfo(args[0], args[1])) return;

	if (strlen(args[0]) == 1) {
		args[0][1] = args[0][0];
		args[0][0] = '0';
	}

	sprintf(argsCommand, "%s %s %s\n", loggedUser.uid, args[0], args[1]);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	receiveUDPMessage();

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);
	if (numTokens != 2 || strcmp(args[0], "RGS") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "OK") == 0) fprintf(stdout, "Group successfully subscribed to.\n");
	else if (strcmp(args[1], "NEW") == 0) fprintf(stdout, "Group successfully created and subscribed to.\n");
	else if (strcmp(args[1], "E_USR") == 0) fprintf(stderr, "error: UID not valid.\n");
	else if (strcmp(args[1], "E_GRP") == 0) fprintf(stderr, "error: GID not valid.\n");
	else if (strcmp(args[1], "E_GNAME") == 0) fprintf(stderr, "error: GName not valid.\n");
	else if (strcmp(args[1], "E_FULL") == 0) fprintf(stderr, "error: new group could not be created.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "Group not subscribed to.\n");
	else exit(1);
}

/*
 * Function: unsub
 * ----------------------------
 *   Executes the unsubscribe command.
 *
 */
void unsub() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "GUR ", argsCommand[MAX_COMMAND_SIZE] = "";

	if (!loggedUser.logged) {
		fprintf(stderr, "error: No logged user.\n");
		return;
	}
	
	numTokens = sscanf(buffer, " %s", args[0]);

	if (numTokens != 1) {
		fprintf(stderr, "error: Incorrect command line arguments\n");
		return;
	}
	if (verifyGroupInfo(args[0], NULL)) return;

	if (strlen(args[0]) == 1) {
		args[0][1] = args[0][0];
		args[0][0] = '0';
	}

	sprintf(argsCommand, "%s %s\n", loggedUser.uid, args[0]);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	receiveUDPMessage();

	numTokens = sscanf(buffer, " %s %s", args[0], args[1]);
	if (numTokens != 2 || strcmp(args[0], "RGU") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "OK") == 0) fprintf(stdout, "Group successfully unsubscribed.\n");
	else if (strcmp(args[1], "E_USR") == 0) fprintf(stderr, "error: UID not valid.\n");
	else if (strcmp(args[1], "E_GRP") == 0) fprintf(stderr, "error: GID not valid.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "Group not unsubscribed.\n");
	else exit(1);
}

/*
 * Function: mgl
 * ----------------------------
 *   Executes the my group list command.
 *
 */
void mgl() {
	int numTokens;
	char args[3][MAX_INFO], command[MAX_COMMAND_SIZE] = "GLM ", argsCommand[MAX_COMMAND_SIZE] = "";

	if (!loggedUser.logged) {
		fprintf(stderr, "error: No logged user.\n");
		return;
	}

	sprintf(argsCommand, "%s\n", loggedUser.uid);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	receiveUDPMessage();

	numTokens = sscanf(buffer, " %s %s %[^\n]", args[0], args[1], buffer);
	if (numTokens < 2 || strcmp(args[0], "RGM") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	}
	else if (strcmp(args[1], "0") == 0) fprintf(stdout, "No groups subscribed to.\n");
	else if (strcmp(args[1], "E_USR") == 0) fprintf(stderr, "error: UID not valid.\n");
	else {
		int n = atoi(args[1]);
		for (int i = 0; i < n; i++) {
			numTokens = sscanf(buffer, " %s %s %s %[^\n]", args[0], args[1], args[2], buffer);
			fprintf(stdout, "Group ID: %s, Group Name: %s, Last Message: %s\n", args[0], args[1], args[2]);
		}
	}	
}

/*
 * Function: sag
 * ----------------------------
 *   Executes the select command.
 *
 */
void sag() {
	int numTokens, errFlag = 0;;
	char args[1][MAX_INFO];

	numTokens = sscanf(buffer, " %s", args[0]);

	if (numTokens != 1) {
		fprintf(stderr, "error: incorrect command arguments\n");
		return;
	}
	if (verifyGroupInfo(args[0], NULL)) return;

	if (strlen(args[0]) == 1) {
		args[0][1] = args[0][0];
		args[0][0] = '0';
	}
	
	selectedGroup.selected = 1;
	strcpy(selectedGroup.gid, args[0]);
	fprintf(stdout, "Group successfully selected.\n");
}

/*
 * Function: sg
 * ----------------------------
 *   Executes the show selected GID command.
 *
 */
void sg() {
	if (!selectedGroup.selected) fprintf(stdout, "No group selected.\n");
	else fprintf(stdout, "Selected GID: %s.\n", selectedGroup.gid);
}
