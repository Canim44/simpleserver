#include "csapp.h"

int main(int argc, char* argv[]){
	int toserverfd, port, varSize, send_length, type = 2; 
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

	int count = 0;
	while ((Rio_readnb(&rio, buf, 256))) {
		printf("%s", buf);
		count++;
	}

	if (count == 1 && buf[0] == '\0') {
		fprintf(stderr, "failed\n");
	}

	Close(toserverfd);
	return 0;
}
