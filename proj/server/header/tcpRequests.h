/*
 * Ficheiro: tcpRequests.h
 * Autor: Luis Freire D'Andrade (N94179), Catarina da Costa Bento (N93230), Bernardo Rosa (N88077)
 * Descricao: [Projeto de RC] Header file of tcpRequests.c.
*/

/*
 * Functions:
*/

void createTcpSocket();
void sendTCPMessage(int socket, char *ptr, int nleft);
int receiveTCPMessage(int socket, char *ptr, int nleft);
void uls(int n);
void pst(int n);
void rtv(int n);
