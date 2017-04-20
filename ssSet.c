#include "csapp.h"

int main(int argc, char** argv){
	int toserverfd, port;
	unsigned int secretkey;
	char *host, *varName, *varValue;
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

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);

	Rio_writen(toserverfd, &secretkey, sizeof(int));
	Rio_readlineb(&rio, buf, MAXLINE);

	Close(toserverfd);
	return 0;
}
