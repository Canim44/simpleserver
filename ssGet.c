#include "csapp.h"

int main(int argc, char* argv[]){
	int toserverfd, port, secretkey, type = 1;
	char *host, *varName;
	char *junk = "cat";
	rio_t rio;
	if (argc != 5) {
		fprintf(stderr, "usage: %s <host> <port> <secretKey> <variableName>\n",
				argv[0]);
		return -1;
	}

	host = argv[1];
	port = atoi(argv[2]);
	secretkey = htonl(atoi(argv[3]));
	varName = argv[4];

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);
	// Send client secretkey to server
	Rio_writen(toserverfd, &secretkey, sizeof(int));
	// Send type of process (GET, 1) to server
	Rio_writen(toserverfd, &type, 1);
	// Send three bytes of padding to server
	Rio_writen(toserverfd, &junk, 3);
	// Send null-terminated variable name to server
	Rio_writen(toserverfd, &varName, 16);

	Close(toserverfd);
	exit(0);
	return 0;
}
