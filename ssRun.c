#include "csapp.h"

int main(int argc, char** argv){
	int toserverfd, port, type = 4;
	unsigned int secretkey;
	char *host, *program;
	char *junk = "cat";
	rio_t rio;
	if (argc != 5) {
		fprintf(stderr, "usage: %s <host> <port> <secretKey> <program>\n",
				argv[0]);
		return -1;
	}

	host = argv[1];
	port = atoi(argv[2]);
	secretkey = htonl(atoi(argv[3]));
	program = argv[4];

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);
	// Send client secretkey to server
	Rio_writen(toserverfd, &secretkey, sizeof(int));
	// Send type of process (RUN, 3) to server
	Rio_writen(toserverfd, &type, 1);
	// Send three bytes of padding to server
	Rio_writen(toserverfd, &junk, 3);
	// Send program to server
	Rio_writen(toserverfd, &program, 9);

	Close(toserverfd);
	return 0;
}
