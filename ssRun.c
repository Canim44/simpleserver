#include "csapp.h"

int main(int argc, char** argv){
	int toserverfd, port;
	unsigned int secretkey;
	char* host, *program;
	char buf[MAXLINE];
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

	Rio_writen(toserverfd, &secretkey, sizeof(int));
	Rio_readnb(&rio, buf, 1);

	if (buf[0] == (char) 1) {
		fprintf(stderr, "failed\n");
		Close(toserverfd);
		return -2;
	}

	Close(toserverfd);
	return 0;
}
