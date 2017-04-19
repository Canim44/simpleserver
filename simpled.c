#include "csapp.h"

void echo(int connfd) { printf("%d", connfd); }

char* varName;
char* varValue;
unsigned int realkey;

int simpleSet(char *MachineName, int TCPport, int SecretKey, char *variableName, char *value, int dataLength) {
	printf("Secret key = %d\n", SecretKey);
	if (SecretKey != realkey) {
		return 0;
	}
	
	printf("Request type = set\n");
	printf("Detail = %s:%s\n", variableName, value);
	printf("Completion = success\n");
	return 0;
}

int simpleGet(char *MachineName, int TCPport, int SecretKey, char *variableName, char *value, int *resultLength) {
	printf("Secret key = %d\n", SecretKey);
	if (SecretKey != realkey) {
		return 0;
	}
	
	printf("Request type = get\n");
	printf("Detail = %s:%s\n", variableName, value);
	printf("Completion = success\n");
	return 0;
}

int simpleDigest(char *MachineName, int TCPport, int SecretKey, char *data, int dataLength, char *result, int *resultLength) {
	printf("Secret key = %d\n", SecretKey);
	if (SecretKey != realkey) {
		return 0;
	}
	
	printf("Request type = digest\n");
	printf("Detail = %s\n", data);
	printf("Completion = success\n");
	return 0;
}

int simpleRun(char *MachineName, int TCPport, int SecretKey, char *request, char *result, int *resultLength) {
	printf("Secret key = %d\n", SecretKey);
	if (SecretKey != realkey) {
		return 0;
	}

	printf("Request type = run\n");
	printf("Detail = %s\n", request);
	printf("Completion = success\n");
	return 0;
}

int main(int argc, char **argv) {
	int listenfd, connfd, port;
	socklen_t clientlen;
	struct sockaddr_in clientaddr;
	struct hostent *hp;
	char *haddrp;
	if (argc != 3) {
		fprintf(stderr, "usage: %s <port> <secretKey>\n", argv[0]);
		exit(0);
	}
	port = atoi(argv[1]);
	realkey = atoi(argv[2]);

	//TODO: input handling for `port` and `secretkey`

	listenfd = Open_listenfd(port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

		/* Determine the domain name and IP address of the client */
		hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		haddrp = inet_ntoa(clientaddr.sin_addr);
		printf("server connected to %s (%s)\n", hp->h_name, haddrp);

		echo(connfd);
		Close(connfd);
	}
	return 0;
}
