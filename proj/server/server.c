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
#include "header/constants.h"
#include "header/udpRequests.h"
#include "header/tcpRequests.h"

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
		if (strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "-p") == 0) strcpy(port, argv[3]);
		else if (strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-v") == 0) strcpy(port, argv[2]);
		else dispArgsError();
	}
	else dispArgsError();
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
                        uls();
                    }
                    else if (strcmp(op, "PST") == 0) {
                    }
                    else if (strcmp(op, "RTV") == 0) {
                    }
                    else {
                        sendTCPMessage(newTcpSocket, "ERR\n", 4);
                    }
                }
                close(newTcpSocket);
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
                    gls();
                }
                else if (strcmp(op, "GSR") == 0) {
                    gsr();
                }
                else if (strcmp(op, "GUR") == 0) {
                    gur();
                }
                else if (strcmp(op, "GLM") == 0) {
                    glm();
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