#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define MAX_INPUT_SIZE 128

int fd, errcode, errno;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[MAX_INPUT_SIZE], port[6], ip[MAX_INPUT_SIZE]; //nosso pc: localhost;

int main(int argc, char *argv[]) {

	//parse command line arguments
	if(argc == 1) {
		if(gethostname(ip, 128) == -1) fprintf(stderr, "error: %s\n", strerror(errno));
		strcpy(port, "58056");
	}
	else if (argc == 5) {
		if(strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-p") == 0) {
			strcpy(ip, argv[2]);
			strcpy(port, argv[4]);
		} 
		else if(strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-n") == 0) {
			strcpy(port, argv[2]);
			strcpy(ip, argv[4]);
		} 
		else fprintf(stderr, "error: incorrect command line arguments\n");
	}
	else if (argc == 3) {
		if(strcmp(argv[1], "-n") == 0) strcpy(ip, argv[2]);
		else if(strcmp(argv[1], "-p") == 0) strcpy(port, argv[2]);
		else fprintf(stderr, "error: incorrect command line arguments\n");
	}
	else fprintf(stderr, "error: incorrect command line arguments\n");

	//sockets creation
	//UDP
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd == -1) exit(1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	errcode = getaddrinfo(ip, port, &hints, &res);
	if(errcode == -1) exit(1);

    while (fgets(buffer, sizeof(buffer)/sizeof(char), stdin)) {
		char op[MAX_INPUT_SIZE], arg1[MAX_INPUT_SIZE], arg2[MAX_INPUT_SIZE];
		int res;

        int numTokens = sscanf(buffer, "%s %s %s", op, arg1, arg2);

        if (numTokens < 1) fprintf(stderr, "error: incorrect command line arguments\n");
        else if(strcmp(op, "reg") == 0) {
			printf("reg\n");
		}
	} 

	freeaddrinfo(res);
	close(fd);

	return 0;
}