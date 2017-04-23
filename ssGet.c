#include "csapp.h"
#define MAXVARVAL 100

int main(int argc, char* argv[]){
	int toserverfd, port, secretkey, length, type = 1;
	char *host, *varName, *varValue = malloc(MAXVARVAL+1);
	char buf[MAXLINE];
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
	Rio_writen(toserverfd, junk, 3);
	// Send null-terminated variable name to server
	Rio_writen(toserverfd, varName, 16);

	// Read three bytes of padding from server
	Rio_readnb(&rio, buf, 3);
	// Read size of value from server
	Rio_readnb(&rio, buf, sizeof(int));
	length = ((buf[0] & 0xFF) << 24) | ((buf[1] & 0xFF) << 16) | 
		((buf[2] & 0xFF) << 8) | (buf[3] & 0xFF);
	// Check if variable found
	if (length == 0) {
		fprintf(stderr, "failed\n");
		return -2;
	}
	// Read value of variable from server
	Rio_readnb(&rio, buf, length);
	strncpy(varValue, buf, length);

	printf("%s\n", varValue);

	Close(toserverfd);
	return 0;
}
