#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "header/util.h"

#define max(A,B) ((A) >= (B) ? (A) : (B))

#define MAX_INPUT_SIZE 3170
#define MAX_MESSAGE_SIZE 240
#define MAX_COMMAND_SIZE 128
#define MAX_IP_SIZE 128
#define MAX_OP_SIZE 4
#define MAX_INFO 26
#define MAX_USER_SUB 600035
#define REPLY_SIZE 8

enum {verbose, quiet} mode;
char buffer[MAX_INPUT_SIZE], port[6];

int udpSocket, tcpSocket, newTcpSocket, errcode, errno;
struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
struct sockaddr_in addr;
ssize_t n;
socklen_t addrlen;

void dispArgsError() {
    fprintf(stderr, "error: incorrect command line arguments\n");
    fprintf(stderr, "Usage: ./DS [-p DSport] [-v]\n");
    exit(1);
}

void parseArgs(int argc, char *argv[]) {
    mode = quiet;
	if (argc == 1) strcpy(port, "58056");
	else if (argc == 2) {
		if (strcmp(argv[1], "-v") == 0) {
            mode = verbose;
            strcpy(port, "58056");
        }
		else dispArgsError();
	}
	else if (argc == 3) {
		if (strcmp(argv[1], "-p") == 0) strcpy(port, argv[2]);
		else dispArgsError();
	}
    else if (argc == 4) {
        mode = verbose;
		if (strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "-p")) strcpy(port, argv[3]);
		else if (strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-v") == 0) strcpy(port, argv[2]);
		else dispArgsError();
	}
	else dispArgsError();
}

void createTcpSocket() {
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (tcpSocket == -1) exit(1);

    memset(&tcpHints, 0, sizeof(tcpHints));
    tcpHints.ai_family = AF_INET;         
    tcpHints.ai_socktype = SOCK_STREAM;
    tcpHints.ai_flags = AI_PASSIVE;

	errcode = getaddrinfo(NULL, port, &tcpHints, &tcpRes);
	if (errcode == -1) exit(1);

    n = bind(tcpSocket, tcpRes->ai_addr, tcpRes->ai_addrlen);
    if (n == -1) exit(1);

    if (listen(tcpSocket, 5) == -1) exit(1);
}

void sendTCPMessage(int socket, char *ptr, int nleft) {
	while (nleft > 0) {
		n = write(socket, ptr, nleft);
		if (n == -1) exit(1);
		else if (n == 0) break;
		nleft -= n;
		ptr += n;
	}
}

void receiveTCPMessage(int socket, char *ptr, int nleft) {
	while (nleft > 0) {
		n = read(socket, ptr, nleft);
		if (n == -1) exit(1);
		else if (n == 0) break;
		nleft -= n;
		ptr += n;
	}
}

void createUdpSocket() {
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1) exit(1);

	memset(&udpHints, 0, sizeof(udpHints));
	udpHints.ai_family = AF_INET;
	udpHints.ai_socktype = SOCK_DGRAM;
    udpHints.ai_flags = AI_PASSIVE;
    
	errcode = getaddrinfo(NULL, port, &udpHints, &udpRes);
	if (errcode == -1) exit(1);
    
    n = bind(udpSocket, udpRes->ai_addr, udpRes->ai_addrlen);
    if (n == 1) exit(1);
}

int createUserDir(char *uid, char *pass) {
    int size;
    FILE * ptr;
    char pathname[30];

    sprintf(pathname,"USERS/%s",uid);
    if ((mkdir(pathname, 0700)) == -1) return 0;

    sprintf(pathname, "USERS/%s/%s_pass.txt", uid, uid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    if (fwrite(pass, sizeof(char), (size = strlen(pass)), ptr) != size) return 0;
    fclose(ptr);

    return 1;
}

int checkUserExists(char *uid) {
    char pathname[25];

    sprintf(pathname, "USERS/%s", uid);
    if (access(pathname, F_OK) == 0) return 1;
    else return 0;
}

void reg() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "RRG ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(verifyUserInfo(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (checkUserExists(args[0])) strcat(reply, "DUP\n");
    else {
        if (!(createUserDir(args[0], args[1]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "REG, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

int delUserDir(char *uid) {
    char pathname[30];

    sprintf(pathname,"USERS/%s/%s_login.txt", uid, uid);
    if (access(pathname, F_OK) == 0)
        if (unlink(pathname) != 0) return 0;

    sprintf(pathname,"USERS/%s/%s_pass.txt", uid, uid);
    if (unlink(pathname) != 0) return 0;

    sprintf(pathname,"USERS/%s",uid);
    if (rmdir(pathname) != 0) return 0;

    return 1;
}

int checkPass(char *uid, char *pass) {
    FILE * ptr;
    char pathname[30], filePass[9];

    sprintf(pathname, "USERS/%s/%s_pass.txt", uid, uid);
    if (!(ptr = fopen(pathname, "r"))) return 0;
    if (8 != fread(filePass, sizeof(char), 8, ptr)) return 0;
    if (strcmp(filePass, pass) != 0) return 0;
    fclose(ptr);

    return 1;
}

void unr() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "RUN ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(verifyUserInfo(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (!(checkUserExists(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkPass(args[0], args[1]))) strcat(reply, "NOK\n");
    else {
        if (!(delUserDir(args[0]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "UNR, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

int createLogFile(char *uid) {
    FILE * ptr;
    char pathname[30];

    sprintf(pathname, "USERS/%s/%s_login.txt", uid, uid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    fclose(ptr);

    return 1;
}

void login() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "RLO ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(verifyUserInfo(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (!(checkUserExists(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkPass(args[0], args[1]))) strcat(reply, "NOK\n");
    else {
        if (!(createLogFile(args[0]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "LOG, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

int checkLog(char *uid) {
    char pathname[30];

    sprintf(pathname, "USERS/%s/%s_login.txt", uid, uid);
    if (access(pathname, F_OK) == 0) return 1;
    else return 0;
}

int deleteLogFile(char *uid) {
    char pathname[30];

    sprintf(pathname,"USERS/%s/%s_login.txt", uid, uid);
    if (unlink(pathname) != 0) return 0;

    return 1;
}

void logout() {
    int numTokens;
    char args[2][MAX_INFO], reply[REPLY_SIZE] = "ROU ";

    numTokens = sscanf(buffer, "%s %s", args[0], args[1]);
    if (numTokens != 2) strcat(reply, "NOK\n");
    else if (!(verifyUserInfo(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (!(checkUserExists(args[0]))) strcat(reply, "NOK\n");
    else if (!(checkPass(args[0], args[1]))) strcat(reply, "NOK\n");
    else if (!(checkLog(args[0]))) strcat(reply, "NOK\n");
    else {
        if (!(deleteLogFile(args[0]))) strcat(reply, "NOK\n");
        else strcat(reply, "OK\n");
    }

    if (mode == verbose) fprintf(stdout, "OUT, UID: %s, IP: %d, PORT: %d\n", args[0], addr.sin_addr.s_addr, addr.sin_port);

    n = sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &addr, addrlen);
	if (n == -1) exit(1);
}

void deleteSockets() {
	freeaddrinfo(udpRes);
	freeaddrinfo(tcpRes);
	close(udpSocket);
	close(tcpSocket);
}

void receiveCommands() {
    char args[3][MAX_INFO], op[MAX_COMMAND_SIZE];
    int counter, maxfd, numTokens;
    pid_t child;
    fd_set rfds;

    FD_ZERO(&rfds);
    maxfd = max(tcpSocket, udpSocket) + 1;
    while (1) {
        FD_SET(tcpSocket, &rfds);
        FD_SET(udpSocket, &rfds);

        counter = select(maxfd, &rfds, NULL, NULL, NULL);
        if (counter <= 0) exit(1);

        if (FD_ISSET(tcpSocket, &rfds)) {
            addrlen = sizeof(addr);
            if ((newTcpSocket = accept(tcpSocket, (struct sockaddr*) &addr, &addrlen)) == -1) exit(1);
            if ((child = fork()) == 0) {
                int origIp = addr.sin_addr.s_addr;
                close(tcpSocket);
                memset(buffer, 0, MAX_INPUT_SIZE);
                receiveTCPMessage(newTcpSocket, buffer, MAX_INPUT_SIZE);
                numTokens = sscanf(buffer, "%s %[^\n]", op, buffer);
                //Switch
                if (numTokens < 1) sendTCPMessage(newTcpSocket, "ERR\n", 4);
                else {
                    if (strcmp(op, "ULS") == 0) {
                    }
                    else if (strcmp(op, "PST") == 0) {
                    }
                    else if (strcmp(op, "RTV") == 0) {
                    }
                    else {
                        sendTCPMessage(newTcpSocket, "ERR\n", 4);
                    }
                }
                closeGLM(newTcpSocket);
                exit(0);
            }
            close(newTcpSocket);
        }
        if (FD_ISSET(udpSocket, &rfds)) {
            addrlen = sizeof(addr);
            memset(buffer, 0, MAX_INPUT_SIZE);
	        n = recvfrom(udpSocket, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
	        if (n == -1) exit(1);
            int origIp = addr.sin_addr.s_addr;
	        numTokens = sscanf(buffer, "%s %[^\n]", op, buffer);
            //Switch
            if (numTokens < 1) {
                n = sendto(udpSocket, "ERR\n", 4, 0, udpRes->ai_addr, udpRes->ai_addrlen);
	            if (n == -1) exit(1);
            }
            else {
                if (strcmp(op, "REG") == 0) {
                    reg();
                }
                else if (strcmp(op, "UNR") == 0) {
                    unr();
                }
                else if (strcmp(op, "LOG") == 0) {
                    login();
                }
                else if (strcmp(op, "OUT") == 0) {
                    logout();
                }
                else if (strcmp(op, "GLS") == 0) {
                }
                else if (strcmp(op, "GSR") == 0) {
                }
                else if (strcmp(op, "GUR") == 0) {
                }
                else if (strcmp(op, "GLM") == 0) {
                }
                else {
                    n = sendto(udpSocket, "ERR\n", 4, 0, udpRes->ai_addr, udpRes->ai_addrlen);
	                if (n == -1) exit(1);
                }
            }
        }
    }
}

void exitServerSession() {
	fprintf(stdout, "Terminating server application.\n");
	deleteSockets();
}

int main(int argc, char *argv[]) {
	parseArgs(argc, argv);
    createTcpSocket();
	createUdpSocket();
	receiveCommands();
	exitServerSession();
	return 0;
}