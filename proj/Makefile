all: 
	
	cd client; gcc util.c udpRequests.c tcpRequests.c user.c -o user;
	cd server; gcc util.c udpRequests.c tcpRequests.c server.c -o DS

servidor: 
	cd server; gcc util.c udpRequests.c tcpRequests.c server.c -o DS

cliente: 
	cd client; gcc util.c udpRequests.c tcpRequests.c user.c -o user

clean:
	cd client; rm user; cd ../server; rm DS