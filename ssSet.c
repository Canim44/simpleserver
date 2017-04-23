#include "csapp.h"

int main(int argc, char** argv){
	int toserverfd, port, varSize, type = 0;
	unsigned int secretkey;
	char *host, *varName, *varValue;
	char *junk = "cat";
	char buf[MAXLINE];
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
	varSize = htonl(strlen(argv[5]));

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);
	// Send client secretkey to server
	Rio_writen(toserverfd, &secretkey, sizeof(int));
	// Send type of process (SET, 0) to server
	Rio_writen(toserverfd, &type, 1);
	// Send three bytes of padding to server
	Rio_writen(toserverfd, junk, 3);
	// Send null-terminated variable name to server
	Rio_writen(toserverfd, varName, 16);
	// Send length of value to server
	if (varSize > 100) {
		varSize = 101;
	}
	Rio_writen(toserverfd, &varSize, sizeof(int));
	// Send value to server
	Rio_writen(toserverfd, varValue, varSize);

	// read success status and 3 bytes of padding
	Rio_readnb(&rio, buf, sizeof(char));
	Rio_readnb(&rio, buf+1, 3*sizeof(char));
	
	if (buf[0] == -1) {
		fprintf(stderr, "failed\n");
		return -2;
	}

	Close(toserverfd);
	return 0;
}
