all: 
	cd client; gcc -w util.c udpRequests.c tcpRequests.c user.c -o ../user;
	cd server; gcc -w util.c udpRequests.c tcpRequests.c server.c -o ../DS;

servidor: 
	cd server; gcc -w util.c udpRequests.c tcpRequests.c server.c -o ../DS;

cliente: 
	cd client; gcc -w util.c udpRequests.c tcpRequests.c user.c -o ../user;

clean:
	rm user; rm DS