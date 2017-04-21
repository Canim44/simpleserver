#include "csapp.h"

int main(int argc, char* argv[]){
	int toserverfd, port, varSize, type = 0; 
	unsigned int secretkey;
	char *host, *value;
	char *junk = "cat";
	rio_t rio;
	if (argc != 5) {
		fprintf(stderr, "usage: %s <host> <port> <secretKey> <value>\n",
				argv[0]);
		return -1;
	}

	host = argv[1];
	port = atoi(argv[2]);
	secretkey = htonl(atoi(argv[3]));
	value = argv[4];
	varSize = sizeof(value);

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);
	// Send client secretkey to server
	Rio_writen(toserverfd, &secretkey, sizeof(int));
	// Send type of process (DIGEST, 2) to server
	Rio_writen(toserverfd, &type, 1);
	// Send three bytes of padding to server
	Rio_writen(toserverfd, &junk, 3);
	// Send length of value
	Rio_writen(toserverfd, &varSize, 4);
	// Send value to server
	Rio_writen(toserverfd, &value, varSize);

	Close(toserverfd);
	exit(0);
	return 0;
}
