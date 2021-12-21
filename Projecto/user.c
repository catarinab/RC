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
// login 84745 00000000
#define MAX_INPUT_SIZE 3170
#define MAX_COMMAND_SIZE 128
#define MAX_IP_SIZE 128
#define MAX_OP_SIZE 12
#define MAX_INFO 26

typedef struct user {
	int logged;
	char uid[6];
	char pwd[9];
} user;

typedef struct group {
	int selected;
	char gid[3];
} group;

int udpSocket, tcpSocket, errcode, errno;
struct addrinfo hints, *udpRes, *tcpRes;
ssize_t n;
socklen_t addrlen;
struct sockaddr_in addr;

char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_IP_SIZE]; //nosso pc: localhost;
user loggedUser; 
group selectedGroup;

void parseArgs(int argc, char *argv[]) {
	if (argc == 1) {
		if (gethostname(ip, 128) == -1) fprintf(stderr, "error: %s\n", strerror(errno));
		strcpy(port, "58056");
	}
	else if (argc == 5) {
		if (strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-p") == 0) {
			strcpy(ip, argv[2]);
			strcpy(port, argv[4]);
		} 
		else if (strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-n") == 0) {
			strcpy(port, argv[2]);
			strcpy(ip, argv[4]);
		} 
		else fprintf(stderr, "error: incorrect command line arguments\n");
	}
	else if (argc == 3) {
		if (strcmp(argv[1], "-n") == 0) strcpy(ip, argv[2]);
		else if (strcmp(argv[1], "-p") == 0) strcpy(port, argv[2]);
		else fprintf(stderr, "error: incorrect command line arguments\n");
	}
	else fprintf(stderr, "error: incorrect command line arguments\n");
}

void createSockets() {
	//UDP
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1) exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	errcode = getaddrinfo(ip, port, &hints, &udpRes);
	if (errcode == -1) exit(1);

	//TCP
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpSocket == -1) exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	errcode = getaddrinfo(ip, port, &hints, &tcpRes);
	if (errcode == -1) exit(1);

}

void deleteSockets() {
	freeaddrinfo(udpRes);
	freeaddrinfo(tcpRes);
	close(udpSocket);
	close(tcpSocket);
}

void concatenateArgs(char *final, char args[][MAX_INFO], int argsNumber) {
	for(int i = 0; i < argsNumber; i++){
		strcat(final, " ");
		strcat(final, args[i]);
	}
	strcat(final, "\n");
}

int verifyUserInfo(char uid[], char pwd[]) {
	int errFlag = 0;
	for (int i = 0; i < strlen(uid); i ++){
		if (isdigit(uid[i]) == 0){
			fprintf(stderr, "error: UID must contain numbers only\n");
			errFlag = 1;
			break;
		}
	}
	if (strlen(uid) != 5){
		fprintf(stderr, "error: UID must have 5 numbers\n");
		errFlag = 1;
	}
	if (strlen(pwd) != 8){
		fprintf(stderr, "error: Password must have 8 characters\n");
		errFlag = 1;
	}
	for (int i = 0; i < strlen(pwd); i ++){
		if (isalnum(pwd[i]) == 0) {
			fprintf(stderr, "error: Password must contain alphanumeric characters only\n");
			errFlag = 1;
			break;
		}
	}
	return errFlag;
}

void reg() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "REG", argsCommand[MAX_COMMAND_SIZE] = "";

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);

	if (numTokens != 2) fprintf(stderr, "error: incorrect command line arguments\n");
	if (verifyUserInfo(args[0], args[1])) return;

	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
	if (numTokens != 2 || strcmp(args[0], "RRG") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "OK") == 0) fprintf(stdout, "User successfully registered.\n");
	else if (strcmp(args[1], "DUP") == 0) fprintf(stdout, "User already registered.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "User not registered.\n");
	else exit(1);
}

void unr() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "UNR", argsCommand[MAX_COMMAND_SIZE] = "";

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);

	if (numTokens != 2) fprintf(stderr, "error: incorrect command line arguments\n");
	if (verifyUserInfo(args[0], args[1])) return;

	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
	if (numTokens != 2 || strcmp(args[0], "RUN") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "OK") == 0) fprintf(stdout, "User successfully unregistered.\n");
	else if (strcmp(args[1], "NOK") == 0) fprintf(stdout, "User not unregistered.\n");
	else exit(1);
}

void resetUser() {
	loggedUser.logged = 0;
	strcpy(loggedUser.uid, "");
	strcpy(loggedUser.pwd, "");
}

void resetGroup() {
	selectedGroup.selected = 0;
	strcpy(selectedGroup.gid, "");
}

void login() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "LOG", argsCommand[MAX_COMMAND_SIZE] = "";

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);

	if (numTokens != 2) fprintf(stderr, "error: incorrect command line arguments\n");
	if (verifyUserInfo(args[0], args[1])) return;

	strcpy(loggedUser.uid, args[0]);
	strcpy(loggedUser.pwd, args[1]);

	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
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

void logout() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "OUT", argsCommand[MAX_COMMAND_SIZE] = "";

	strcpy(args[0], loggedUser.uid);
	strcpy(args[1], loggedUser.pwd);
	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
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

void su() {
	if (loggedUser.logged) fprintf(stdout, "UID of logged user: %s.\n", loggedUser.uid);
	else fprintf(stdout, "No logged user.\n");
}

void gl() {
	int numTokens;
	char args[3][MAX_INFO], command[MAX_COMMAND_SIZE] = "GLS\n", argsCommand[MAX_COMMAND_SIZE] = "";

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s %[^\n]", args[0], args[1], buffer);
	if (numTokens < 2 || strcmp(args[0], "RGL") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	} 
	else if (strcmp(args[1], "0") == 0) fprintf(stdout, "No groups available.\n");
	else {
		int n = atoi(args[1]);
		for (int i = 0; i < n; i++) {
			numTokens = sscanf(buffer, "%s %s %s %[^\n]", args[0], args[1], args[2], buffer);
			fprintf(stdout, "Group ID: %s, Group Name: %s\n", args[0], args[1]);
		}
	}
}

void sub() {
	int numTokens;
	char args[3][MAX_INFO], command[MAX_COMMAND_SIZE] = "GSR", argsCommand[MAX_COMMAND_SIZE] = "";

	if (!loggedUser.logged) fprintf(stdout, "warning: No logged user.\n");

	numTokens = sscanf(buffer, "%s %s", args[1], args[2]);

	if (numTokens != 2) {
		fprintf(stderr, "error: incorrect command line arguments\n");
		exit(1);
	}

	if (strlen(args[1]) == 1) {
		args[1][1] = args[1][0];
		args[1][0] = '0';
	}

	strcpy(args[0], loggedUser.uid);
	concatenateArgs(argsCommand, args, 3);
	strcat(command, argsCommand);

	printf(command);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
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

void unsub() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "GUR", argsCommand[MAX_COMMAND_SIZE] = "";

	if (!loggedUser.logged) fprintf(stdout, "warning: No logged user.\n");

	numTokens = sscanf(buffer, "%s", args[1]);

	if (numTokens != 1) {
		fprintf(stderr, "error: incorrect command line arguments\n");
		exit(1);
	}

	if (strlen(args[1]) == 1) {
		args[1][1] = args[1][0];
		args[1][0] = '0';
	}

	strcpy(args[0], loggedUser.uid);
	concatenateArgs(argsCommand, args, 2);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
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

void mgl() {
	int numTokens;
	char args[3][MAX_INFO], command[MAX_COMMAND_SIZE] = "GLM", argsCommand[MAX_COMMAND_SIZE] = "";

	if (!loggedUser.logged) fprintf(stdout, "warning: No logged user.\n");

	strcpy(args[0], loggedUser.uid);
	concatenateArgs(argsCommand, args, 1);
	strcat(command, argsCommand);

	n = sendto(udpSocket, command, strlen(command), 0, udpRes->ai_addr, udpRes->ai_addrlen);
	if (n == -1) exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	if (n == -1) exit(1);

	numTokens = sscanf(buffer, "%s %s %[^\n]", args[0], args[1], buffer);
	if (numTokens < 2 || strcmp(args[0], "RGM") != 0){
		fprintf(stderr, "error: Server Error.\n");
		exit(1);
	}
	else if (strcmp(args[1], "0") == 0) fprintf(stdout, "No groups subscribed to.\n");
	else if (strcmp(args[1], "E_USR") == 0) fprintf(stderr, "error: UID not valid.\n");
	else {
		int n = atoi(args[1]);
		for (int i = 0; i < n; i++) {
			numTokens = sscanf(buffer, "%s %s %s %[^\n]", args[0], args[1], args[2], buffer);
			fprintf(stdout, "Group ID: %s, Group Name: %s\n", args[0], args[1]);
		}
	}	
}

void sag() {
	int numTokens, errFlag = 0;;
	char args[1][MAX_INFO];

	numTokens = sscanf(buffer, "%s", args[0]);

	if (numTokens != 1) {
		fprintf(stderr, "error: incorrect command line arguments\n");
		exit(1);
	}
	if (strlen(args[0]) > 2){
		fprintf(stderr, "error: GID must have 1 or 2 numbers\n");
		errFlag = 1;
	}
	for (int i = 0; i < strlen(args[0]); i++){
		if (isdigit(args[0][i]) == 0){
			fprintf(stderr, "error: GID must contain numbers only\n");
			errFlag = 1;
			break;
		}
	}
	if (errFlag) return;

	if (strlen(args[0]) == 1) {
		args[0][1] = args[0][0];
		args[0][0] = '0';
	}
	
	selectedGroup.selected = 1;
	strcpy(selectedGroup.gid, args[0]);
	fprintf(stdout, "Group successfully selected.\n");
}

void sg() {
	if (!selectedGroup.selected) fprintf(stdout, "No group selected.\n");
	else fprintf(stdout, "Selected GID: %s.\n", selectedGroup.gid);
}

void ul() {
	int numTokens;
	char args[2][MAX_INFO], command[MAX_COMMAND_SIZE] = "ULS", argsCommand[MAX_COMMAND_SIZE] = "";

	if (!selectedGroup.selected) {
		fprintf(stdout, "warning: No group selected.\n");
		return;
	}

	strcpy(args[0], selectedGroup.gid);
	concatenateArgs(argsCommand, args, 1);
	strcat(command, argsCommand);
}

void readCommands() {
	resetUser();
	resetGroup();
	
	while (fgets(buffer, sizeof(buffer)/sizeof(char), stdin)) {
		char op[MAX_OP_SIZE];

        int numTokens = sscanf(buffer, "%s %[^\n]", op, buffer);

        if (numTokens < 1) fprintf(stderr, "error: incorrect command line arguments\n");
		else {
			if (strcmp(op, "reg") == 0) {
				reg();
			}
			else if (strcmp(op, "unr") == 0 || strcmp(op, "unregister") == 0) {
				unr();
			}
			else if (strcmp(op, "login") == 0) {
				login();
			}
			else if (strcmp(op, "logout") == 0) {
				logout();
			}
			else if (strcmp(op, "su") == 0 || strcmp(op, "showuid") == 0) {
				su();
			}
			else if (strcmp(op, "exit") == 0) {
				return;
			}
			else if (strcmp(op, "gl") == 0 || strcmp(op, "groups") == 0) {
				gl();
			}
			else if (strcmp(op, "s") == 0 || strcmp(op, "subscribe") == 0) {
				sub();
			}
			else if (strcmp(op, "u") == 0 || strcmp(op, "unsubscribe") == 0) {
				unsub();
			}
			else if (strcmp(op, "mgl") == 0 || strcmp(op, "my_groups") == 0) {
				mgl();
			}
			else if (strcmp(op, "sag") == 0 || strcmp(op, "select") == 0) {
				sag();
			}
			else if (strcmp(op, "sg") == 0 || strcmp(op, "showgid") == 0) {
				sg();
			}
			else if(strcmp(op, "ulist") == 0 || strcmp(op, "ul") == 0) {
				ul();
			}
			else fprintf(stdout, "Operation not recognized.\n");
		}
	}
}

void exitClientSession() {
	fprintf(stdout, "Terminating user application.\n");
	resetUser();
	resetGroup();
	deleteSockets();
}

int main(int argc, char *argv[]) {
	parseArgs(argc, argv);
	createSockets();
	readCommands();
	exitClientSession();
	return 0;
}