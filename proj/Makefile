all: 
	cd client; gcc -w util.c udpRequests.c tcpRequests.c user.c -o ../user;
	cd server; gcc -w util.c udpRequests.c tcpRequests.c server.c -o ../DS;

server: 
	cd server; gcc -w util.c udpRequests.c tcpRequests.c server.c -o ../DS;

client: 
	cd client; gcc -w util.c udpRequests.c tcpRequests.c user.c -o ../user;

clean:
	rm user; rm DS