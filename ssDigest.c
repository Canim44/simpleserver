#include "csapp.h"

int main(int argc, char* argv[]){
	int toserverfd, port, varSize, send_length, size, type = 2; 
	unsigned int secretkey;
	char *host, *value;
	char *junk = "cat";
	char buf[MAXLINE];
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
	varSize = htonl(strlen(argv[4]));
	if (strlen(argv[4]) > 100) {
		send_length = 100;
	}
	else {
		send_length = strlen(argv[4]);
	}

	toserverfd = open_clientfd(host, port);
	Rio_readinitb(&rio, toserverfd);
	// Send client secretkey to server
	Rio_writen(toserverfd, &secretkey, sizeof(int));
	// Send type of process (DIGEST, 2) to server
	Rio_writen(toserverfd, &type, 1);
	// Send three bytes of padding to server
	Rio_writen(toserverfd, junk, 3);
	// Send length of value
	Rio_writen(toserverfd, &varSize, sizeof(int));
	// Send value to server
	Rio_writen(toserverfd, value, send_length);

	// Read success status and 3 bytes of padding from server
	Rio_readnb(&rio, buf, sizeof(char));
	Rio_readnb(&rio, buf+1, 3*sizeof(char));

	if (buf[0] == -1) {
		fprintf(stderr, "failed\n");
		return -2;
	}
	// Read size of value from server
	printf("Reading size\n");
	Rio_readnb(&rio, buf+4, sizeof(int));
	size = ((buf[4] & 0xFF) << 24) | ((buf[5] & 0x55) << 16) |
		((buf[6] & 0xFF) << 8) | (buf[7] & 0xFF);
	printf("size: %d\n", size);
	// Display value to client
	if (size != 0) {
		printf("Reading value\n");
		Rio_readnb(&rio, buf+8, size);
		printf("%s\n", buf+8);
	}

	Close(toserverfd);
	return 0;
}
