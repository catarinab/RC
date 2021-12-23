#include "header/util.h"
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

void createTCPSocket() {
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpSocket == -1) exit(1);

	memset(&tcpHints, 0, sizeof(tcpHints));
	tcpHints.ai_family = AF_INET;
	tcpHints.ai_socktype = SOCK_STREAM;

	errcode = getaddrinfo(ip, port, &tcpHints, &tcpRes);
	if (errcode == -1) exit(1);
	n=connect(tcpSocket,tcpRes->ai_addr,tcpRes->ai_addrlen);
	if(n==-1)exit(1);
}

void ul() {
	int numTokens, nleft = MAX_USER_SUB;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "ULS ";
	char responseBuffer[MAX_USER_SUB], * ptr = responseBuffer, groupName[MAX_INFO]; // fazer as contas do tamanho
	if (!selectedGroup.selected) {
		fprintf(stdout, "warning: No group selected.\n");
		return;
	}
	createTCPSocket();

	strcat(command, selectedGroup.gid);
	strcat(command, "\n");

	n= write(tcpSocket,command,strlen(command));
	if(n==-1) exit(1);

	memset(responseBuffer, 0, MAX_USER_SUB);
	while(nleft>0) {
		n = read(tcpSocket,ptr,nleft);
		if(n==-1) exit(1);
		else if(n==0) break;
		nleft-=n; ptr+=n;
	}
	//freeaddrinfo(tcpRes);
	close(tcpSocket);

	numTokens = sscanf(responseBuffer, "%s %s %s%[^\n]", args[0], args[1], groupName, responseBuffer);
	if (numTokens < 2 || strcmp(args[0], "RUL") != 0) fprintf(stderr, "error: Server Error.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "The group does not exist.\n");
	else if (strcmp(args[1], "OK") == 0){
		if(numTokens == 3) fprintf(stdout, "No Users subscribed to group %s.\n", groupName);
		else fprintf(stdout, "Users subscribed to group %s:%s\n", groupName, responseBuffer);
	} 
	else fprintf(stderr, "error: Server Error.\n");
}
