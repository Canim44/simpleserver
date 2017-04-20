#include "csapp.h"

int main(int argc, char* argv[]){
	int toserverfd, port, secretkey;
	char* host, program;
	rio_t rio;
	if (argc != 5) {
		fprintf(stderr, "usage: %s <host> <port> <secretKey> <program>\n",
				argv[0]);
		return -1;
	}

	host = argv[1];
	port = atoi(argv[2]);
	secretkey = atoi(argv[3]);
	program = argv[4];

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);
	
	Close(toserverfd);
	exit(0);
	return 0;
}
