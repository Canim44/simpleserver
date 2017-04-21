#include "csapp.h"

int main(int argc, char** argv){
	int toserverfd, port, varSize, type = 0;
	unsigned int secretkey;
	char *host, *varName, *varValue;
	char *junk = "cat";
	rio_t rio;
	if (argc != 6) {
		fprintf(stderr, "usage: %s <host> <port> <secretKey> <variableName> <variableValue>\n",
				argv[0]);
		return -1;
	}

	host = argv[1];
	port = atoi(argv[2]);
	secretkey = htonl(atoi(argv[3]));
	varName = argv[4];
	varValue = argv[5];
	varSize = htonl(sizeof(varValue));

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);
	// Send client secretkey to server
	Rio_writen(toserverfd, &secretkey, sizeof(int));
	// Send type of process (SET, 0) to server
	Rio_writen(toserverfd, &type, 1);
	// Send three bytes of padding to server
	Rio_writen(toserverfd, &junk, 3);
	// Send null-terminated variable name to server
	Rio_writen(toserverfd, &varName, 16);
	// Send length of value to server
	Rio_writen(toserverfd, &varSize, sizeof(int));
	// Send value to server
	Rio_writen(toserverfd, &varValue, varSize);

	Close(toserverfd);
	return 0;
}
