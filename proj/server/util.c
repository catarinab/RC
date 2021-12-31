#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int verifyDigit(char buff[], int beg, int end) {
	for (int i = beg; i < end; i ++){
		if (isdigit(buff[i]) == 0) return 0;
    }
	return 1;
}

int verifyAlnum(char buff[], int beg, int end) {
	for (int i = beg; i < end; i ++){
		if (isalnum(buff[i]) == 0) return 0;
	}
	return 1;
}

int verifyUserInfo(char uid[], char pwd[]) {
	if (strlen(uid) != 5 || strlen(pwd) != 8 || !verifyDigit(uid, 0, strlen(uid)) || !verifyAlnum(pwd, 0, strlen(pwd))) 
        return 0;
	return 1;
}