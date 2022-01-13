/*
 * Ficheiro: util.c
 * Autor: Luis Freire D'Andrade (N94179), Catarina da Costa Bento (N93230), Bernardo Rosa (N88077)
 * Descricao: [Projeto de RC] Development, in C language, of useful function used in the execution of server commands.
*/

/*
 * Libraries:
*/

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
#include "header/constants.h"

/*
 * Global Variables:
*/

int udpSocket, tcpSocket, newTcpSocket, errcode, errno;
struct addrinfo udpHints, tcpHints, *udpRes, *tcpRes;
struct sockaddr_in addr;
ssize_t n;
socklen_t addrlen;
char userIP[INET_ADDRSTRLEN];

char buffer[MAX_INPUT_SIZE], port[6];

enum {verbose, quiet} mode;

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
 *
 *   returns: boolean related to errors.
 */
int verifyDigit(char buff[], int beg, int end) {
	for (int i = beg; i < end; i ++){
		if (isdigit(buff[i]) == 0) return 0;
    }
	return 1;
}

/*
 * Function: verifyAlnum
 * ----------------------------
 *   Checks if the string only contains alphanumeric characters.
 *
 *   buff: buffer where the string to check is.
 *   beg: index where the string begins.
 *   end: index where the string ends.
 *
 *   returns: boolean related to errors.
 */
int verifyAlnum(char buff[], int beg, int end) {
	for (int i = beg; i < end; i ++){
		if (isalnum(buff[i]) == 0) return 0;
	}
	return 1;
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
	if (strlen(uid) != 5 || strlen(pwd) != 8 || !verifyDigit(uid, 0, strlen(uid)) || !verifyAlnum(pwd, 0, strlen(pwd))) 
        return 0;
	return 1;
}

/*
 * Function: createUserDir
 * ----------------------------
 *   Creates a directory for an user.
 *
 *   uid: user identification.
 *   pwd: user password.
 *
 *   returns: boolean related to errors.
 */
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

/*
 * Function: checkUserExists
 * ----------------------------
 *   Checks if an user exists.
 *
 *   uid: user identification.
 *
 *   returns: boolean related to errors.
 */
int checkUserExists(char *uid) {
    char pathname[25];

    sprintf(pathname, "USERS/%s", uid);
    if (access(pathname, F_OK) == 0) return 1;
    else return 0;
}

/*
 * Function: countGroups
 * ----------------------------
 *   Counts the existent groups.
 *
 *   returns: number of groups.
 */
int countGroups() {
    int nDir = 0;
    char pathname[20];
    
    strcpy(pathname, "GROUPS/01");
    while (access(pathname, F_OK) == 0) {
        nDir++;   
        if (nDir < 9) sprintf(pathname, "GROUPS/0%d", (nDir + 1));
        else sprintf(pathname, "GROUPS/%d", (nDir + 1));
    }
    return nDir;
}

/*
 * Function: countMessages
 * ----------------------------
 *   Counts the existent messages in a group starting from a specified message.
 *
 *   gid: group identification.
 *   mid: message identification that the function counts from.
 *
 *   returns: number of messages.
 */
int countMessages(char *gid, int mid) {
    int nMsg = 0;
    char pathname[20];

    if (mid < 9) sprintf(pathname, "GROUPS/%s/MSG/000%d", gid, (mid + 1));
    else if (mid < 99) sprintf(pathname, "GROUPS/%s/MSG/00%d", gid, (mid + 1));
    else if (mid < 999) sprintf(pathname, "GROUPS/%s/MSG/0%d", gid, (mid + 1));
    else sprintf(pathname, "GROUPS/%s/MSG/%d", gid, (mid + 1));
    while (access(pathname, F_OK) == 0) {
        nMsg++;
        mid++;
        if (mid < 9) sprintf(pathname, "GROUPS/%s/MSG/000%d", gid, (mid + 1));
        else if (mid < 99) sprintf(pathname, "GROUPS/%s/MSG/00%d", gid, (mid + 1));
        else if (mid < 999) sprintf(pathname, "GROUPS/%s/MSG/0%d", gid, (mid + 1));
        else sprintf(pathname, "GROUPS/%s/MSG/%d", gid, (mid + 1));
    }
    return nMsg;
}

/*
 * Function: delUserDir
 * ----------------------------
 *   Deletes the directory of an user.
 *
 *   uid: user identification.
 *
 *   returns: boolean related to errors.
 */
int delUserDir(char *uid) {
    int nGroups;
    char pathname[30], gid[3];

    sprintf(pathname,"USERS/%s/%s_login.txt", uid, uid);
    if (access(pathname, F_OK) == 0)
        if (unlink(pathname) != 0) return 0;

    sprintf(pathname,"USERS/%s/%s_pass.txt", uid, uid);
    if (unlink(pathname) != 0) return 0;

    nGroups = countGroups();
    if (nGroups > 0) {
        for (int i = 1; i <= nGroups; i++) {
            if (i < 10) sprintf(gid, "0%d", i);
            else sprintf(gid, "%d", i);
            sprintf(pathname, "GROUPS/%s/%s.txt", gid, uid);
            if (access(pathname, F_OK) == 0)
                if (unlink(pathname) != 0) return 0;
        }
    }

    sprintf(pathname,"USERS/%s",uid);
    if (rmdir(pathname) != 0) return 0;

    return 1;
}

/*
 * Function: checkPass
 * ----------------------------
 *   Checks if a password matches an user's password.
 *
 *   uid: user identification.
 *   pass: password.
 *
 *   returns: boolean related to errors.
 */
int checkPass(char *uid, char *pass) {
    FILE * ptr;
    char pathname[30], filePass[9];

    sprintf(pathname, "USERS/%s/%s_pass.txt", uid, uid);
    if (!(ptr = fopen(pathname, "r"))) return 0;
    memset(filePass, 0, 9);
    if (8 != fread(filePass, sizeof(char), 8, ptr)) return 0;
    if (strcmp(filePass, pass) != 0) return 0;
    fclose(ptr);

    return 1;
}

/*
 * Function: createLogFile
 * ----------------------------
 *   Creates a file indicating that a user is logged in.
 *
 *   uid: user identification.
 *
 *   returns: boolean related to errors.
 */
int createLogFile(char *uid) {
    FILE * ptr;
    char pathname[30];

    sprintf(pathname, "USERS/%s/%s_login.txt", uid, uid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    fclose(ptr);

    return 1;
}

/*
 * Function: checkLog
 * ----------------------------
 *   Checks if a user is logged in.
 *
 *   uid: user identification.
 *
 *   returns: boolean related to errors.
 */
int checkLog(char *uid) {
    char pathname[30];

    sprintf(pathname, "USERS/%s/%s_login.txt", uid, uid);
    if (access(pathname, F_OK) == 0) return 1;
    else return 0;
}

/*
 * Function: deleteLogFile
 * ----------------------------
 *   Deletes the file indicating that the user is logged in.
 *
 *   uid: user identification.
 *
 *   returns: boolean related to errors.
 */
int deleteLogFile(char *uid) {
    char pathname[30];

    sprintf(pathname,"USERS/%s/%s_login.txt", uid, uid);
    if (unlink(pathname) != 0) return 0;

    return 1;
}

/*
 * Function: checkGroup
 * ----------------------------
 *   Checks if a group exists.
 *
 *   gid: group identification.
 *
 *   returns: boolean related to errors.
 */
int checkGroup(char *gid) {
    char pathname[10];

    if (strcmp(gid, "00") == 0) {
        if (access("GROUPS/99", F_OK) == 0) return 99;
        else return 1;
    }
    else {
        sprintf(pathname, "GROUPS/%s", gid);
        if (access(pathname, F_OK) == 0) return 1;
        else return 0;
    }
}

/*
 * Function: checkGroupInfo
 * ----------------------------
 *   Checks if a name matches a group's name.
 *
 *   gid: group identification.
 *   gname: name.
 *
 *   returns: boolean related to errors.
 */
int checkGroupInfo(char *gid, char *gname) {
    FILE * ptr;
    int nDir = 0;
    char pathname[25], fileName[25];
    
    if (strcmp(gid, "00") == 0) return 1;

    sprintf(pathname, "GROUPS/%s/%s_name.txt", gid, gid);
    if (!(ptr = fopen(pathname, "r"))) return 0;
    memset(fileName, 0, 25);
    if (0 >= fread(fileName, sizeof(char), 24, ptr)) return 0;
    if (strcmp(fileName, gname) != 0) return 0;
    fclose(ptr);

    return 1;
}

/*
 * Function: createGroupDir
 * ----------------------------
 *   Creates a directory for a group.
 *
 *   gid: group identification.
 *   gname: group name.
 *
 *   returns: boolean related to errors.
 */
int createGroupDir(char *gid, char *gname) {
    int size;
    FILE * ptr;
    char pathname[25];

    sprintf(pathname, "GROUPS/%s", gid);
    if ((mkdir(pathname, 0700)) == -1) return 0;
    sprintf(pathname, "GROUPS/%s/MSG", gid);
    if ((mkdir(pathname, 0700)) == -1) return 0;

    sprintf(pathname, "GROUPS/%s/%s_name.txt", gid, gid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    if (fwrite(gname, sizeof(char), (size = strlen(gname)), ptr) != size) return 0;
    fclose(ptr);

    return 1;
}

/*
 * Function: createSubFile
 * ----------------------------
 *   Creates a file indicating that a user is subscribed to a group.
 *
 *   uid: user identification.
 *   gid: group identification.
 *
 *   returns: boolean related to errors.
 */
int createSubFile(char *uid, char *gid) {
    FILE * ptr;
    char pathname[20];

    sprintf(pathname, "GROUPS/%s/%s.txt", gid, uid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    fclose(ptr);

    return 1;
}

/*
 * Function: deleteSubFile
 * ----------------------------
 *   Deletes the file indicating that an user is subscribed to a group.
 *
 *   uid: user identification.
 *   gid: group identification.
 *
 *   returns: boolean related to errors.
 */
int deleteSubFile(char *uid, char *gid) {
    char pathname[20];

    sprintf(pathname,"GROUPS/%s/%s.txt", gid, uid);
    if (unlink(pathname) != 0) return 0;

    return 1;
}

/*
 * Function: verifyUserFile
 * ----------------------------
 *   Checks if a given file is a user subscription file, and if so returns the UID by reference.
 *
 *   userFile: given file.
 *   uid: pointer to return the user identification to.
 *
 *   returns: boolean related to errors.
 */
int verifyUserFile(char *userFile, char *uid) {
    char ext[4];
    char *token;

    memset(uid, 0, MAX_INFO);
    token = strtok(userFile, ".");
    strcpy(uid, token);
    token = strtok(NULL, ".");
    strcpy(ext, token);
    if (verifyDigit(uid, 0, strlen(uid)) && strlen(uid) == 5 && strcmp(ext, "txt") == 0) return 1;
    else return 0;
}

/*
 * Function: checkMessage
 * ----------------------------
 *   Checks if a message exists in a group or, if the input is 0000, if there's 
 *   space for another one and, if so, returns a MID by reference.
 *
 *   gid: group identification.
 *   mid: message identification.
 *
 *   returns: boolean related to errors.
 */
int checkMessage(char *gid, char *mid) {
    int nMsg;
    char pathname[20];

    if (strcmp(mid, "0000") == 0) {
        sprintf(pathname, "GROUPS/%s/MSG/9999", gid);
        if (access(pathname, F_OK) == 0) return 0;
        else {
            nMsg = countMessages(gid, 0);
            nMsg++;
            if (nMsg < 10) sprintf(mid, "000%d", nMsg);
            else if (nMsg < 100) sprintf(mid, "00%d", nMsg);
            else if (nMsg < 1000) sprintf(mid, "0%d", nMsg);
            else sprintf(mid, "%d", nMsg);
            return 1;
        }
    }
    else {
        sprintf(pathname, "GROUPS/%s/MSG/%s", gid, mid);
        if (access(pathname, F_OK) == 0) return 1;
        else return 0;
    }
}

/*
 * Function: createMsgDir
 * ----------------------------
 *   Creates a directory for a message.
 *
 *   uid: user that posted the message.
 *   gid: group to where the message was posted.
 *   mid: message identification.
 *   text: message.
 *
 *   returns: boolean related to errors.
 */
int createMsgDir(char *uid, char *gid, char *mid, char *text) {
    int size;
    FILE * ptr;
    char pathname[35];

    sprintf(pathname,"GROUPS/%s/MSG/%s", gid, mid);
    if ((mkdir(pathname, 0700)) == -1) return 0;

    sprintf(pathname, "GROUPS/%s/MSG/%s/A U T H O R.txt", gid, mid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    if (fwrite(uid, sizeof(char), (size = strlen(uid)), ptr) != size) return 0;
    fclose(ptr);

    sprintf(pathname, "GROUPS/%s/MSG/%s/T E X T.txt", gid, mid);
    if (!(ptr = fopen(pathname, "w"))) return 0;
    if (fwrite(text, sizeof(char), (size = strlen(text)), ptr) != size) return 0;
    fclose(ptr);

    return 1;
}

/*
 * Function: checkSub
 * ----------------------------
 *   Checks if a user is subscribed to a group.
 *
 *   uid: user identification.
 *   gid: group identification.
 *
 *   returns: boolean related to errors.
 */
int checkSub(char * uid, char * gid) {
    char pathname[20];

    sprintf(pathname, "GROUPS/%s/%s.txt", gid, uid);
    if (access(pathname, F_OK) == 0) return 1;
    else return 0;
}