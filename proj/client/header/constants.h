/*
 * Ficheiro: constants.h
 * Autor: Luis Freire D'Andrade (N94179), Catarina da Costa Bento (N93230), Bernardo Rosa (N88077)
 * Descricao: [Projeto de RC] Header file containing all constants and structures used in the User Application.
*/

/*
 * Constants:
*/

#define MAX_INPUT_SIZE 3170
#define MAX_MESSAGE_SIZE 245
#define MAX_COMMAND_SIZE 128
#define MAX_IP_SIZE 128
#define MAX_OP_SIZE 12
#define MAX_INFO 26
#define MAX_USER_SUB 600035 

/*
 * Structures:
*/

typedef struct user {
	int logged;
	char uid[6];
	char pwd[9];
} user;

typedef struct group {
	int selected;
	char gid[3];
} group;