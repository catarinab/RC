#define MAX_INPUT_SIZE 3170
#define MAX_MESSAGE_SIZE 245
#define MAX_COMMAND_SIZE 128
#define MAX_IP_SIZE 128
#define MAX_OP_SIZE 12
#define MAX_INFO 26
#define MAX_USER_SUB 600035 

typedef struct user {
	int logged;
	char uid[6];
	char pwd[9];
} user;

typedef struct group {
	int selected;
	char gid[3];
} group;