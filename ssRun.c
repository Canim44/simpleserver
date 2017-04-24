#include "csapp.h"

int main(int argc, char** argv){
	int toserverfd, port, type = 3;
	int size = 0;
	unsigned int secretkey;
	char *host, *program;
	char *junk = "cat";
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
	// Send client secretkey to server
	Rio_writen(toserverfd, &secretkey, sizeof(int));
	// Send type of process (RUN, 3) to server
	Rio_writen(toserverfd, &type, 1);
	// Send three bytes of padding to server
	Rio_writen(toserverfd, junk, 3);
	// Send program to server
	Rio_writen(toserverfd, program, 8);
	
	// Read success status and three bytes of padding from server
	Rio_readnb(&rio, buf, sizeof(char));
	Rio_readnb(&rio, buf+1, 3*sizeof(char));

	if (buf[0] == -1) {
		fprintf(stderr, "failed\n");
		return -2;
	}
	// Read size of value from server
	Rio_readnb(&rio, buf+4, sizeof(int));
	size = ((buf[4] & 0xFF) << 24) | ((buf[5] & 0xFF) << 16) | 
		((buf[6] & 0xFF) << 8) | (buf[7] & 0xFF);
	// Display value to client
	if (size != 0) {
		Rio_readnb(&rio, buf+8, size);
		printf("%s\n", buf+8);
	}

	Close(toserverfd);
	return 0;
}
