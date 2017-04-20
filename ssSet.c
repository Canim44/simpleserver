#include "csapp.h"

int main(int argc, char** argv){
	int toserverfd, port, secretkey;
	char* host, varName, varValue;
	rio_t rio;
	if (argc != 6) {
		fprintf(stderr, "usage: %s <host> <port> <secretKey> <variableName> <variableValue>\n",
				argv[0]);
		return -1;
	}

	host = argv[1];
	port = atoi(argv[2]);
	secretkey = atoi(argv[3]);
	varName = argv[4];
	varValue = argv[5];

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);
	
	Close(toserverfd);
	return 0;
}
