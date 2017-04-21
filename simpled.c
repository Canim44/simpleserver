#include "csapp.h"
#define MAXVARLEN 128

void echo(int connfd) { printf("%d", connfd); }

char** varName;		// environment variable names
char** varValue;	// environment variable values
int varSize = 10;	// default size of var array (flexible)
int nVars = 0;		// index of last set variable value
unsigned int realkey;	// actual secret key value to access server

int simpleSet(char *variableName, char *value, int dataLength) {
	printf("Request type = set\n");

	// check if more space is needed; allocate more if so
	if (varSize < nVars) {
		varSize += 10;
		varName = realloc(varName, varSize * (MAXVARLEN+1));
		varValue = realloc(varValue, varSize * (MAXVARLEN+1));
		if (!varName || !varValue) {
			fprintf(stderr, "allocation error\n");
			exit(EXIT_FAILURE);
		}
	}

	int index = -1;
	// find index of var name (if it exists)
	int i;
	for (i = 0; i < varSize; i++) {
		if (strcmp(variableName, varName[i]) == 0) {
			index = i;
			break;
		}
	}
	// otherwise, put variable in next slot in array
	if (index == -1) {
		index = nVars;
		nVars++;
	}
	
	// set var name and value
	strcpy(varName[index], variableName);
	strcpy(varValue[index], value);

	printf("Detail = %s: %s\n", variableName, value);
	printf("Completion = success\n");
	return 0;
}

int simpleGet(char *variableName, char *value) {
	printf("Request type = get\n");

	int i;
	int index = -1;
	for (i = 0; i < nVars; i++) {
		if (strcmp(varName[i], variableName) == 0) {
			index = i;
			break;
		}
	}
	
	if (index != -1) {
		printf("Detail = %s:%s\n", variableName, value);
		printf("Completion = success\n");
		return 0;
	}

	printf("Could not locate variable %s\n", variableName);
	return -2;
}

int simpleDigest(char *data, int dataLength, char *result) {
	printf("Request type = digest\n");

	char* command = strcat("sh -c \'echo `/bin/hostname` ", data); 
	command = strcat(command, " | /usr/bin/md5sum");
	system(command);

	printf("Detail = %s\n", data);
	printf("Completion = success\n");
	return 0;
}

int simpleRun(char *request, char *result) {
	printf("Request type = run\n");

	if (strcmp(request, "inet") == 0) {
		system("/sbin/ifconfig -a");
	}
	else if (strcmp(request, "hosts") == 0) {
		system("/bin/cat /etc/hosts");
	}
	else if (strcmp(request, "uptime") == 0) {
		system("/usr/bin/uptime");
	}
	else {
		printf("Command not found\n");
		return -2;
	}

	printf("Detail = %s\n", request);
	printf("Completion = success\n");
	return 0;
}

int main(int argc, char **argv) {
	int listenfd, connfd, port, type, i;
	socklen_t clientlen;
	struct sockaddr_in clientaddr;
	struct hostent *hp;
	char *haddrp;
	char *clientbuf[MAXLINE];
	char buf[MAXLINE];
	unsigned int clientkey;
	rio_t rio;
	if (argc != 3) {
		fprintf(stderr, "usage: %s <port> <secretKey>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[1]);
	realkey = atoi(argv[2]);

	// allocate environment variables
	varName = malloc(varSize);
	varValue = malloc(varSize);
	for (i = 0; i < varSize; i++) {
		varName[i] = malloc(MAXVARLEN+1);
		varValue[i] = malloc(MAXVARLEN+1);
	}

	listenfd = Open_listenfd(port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

		// Determine the domain name and IP address of the client
		hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		haddrp = inet_ntoa(clientaddr.sin_addr);
		printf("server connected to %s (%s)\n", hp->h_name, haddrp);
		
		// Get client secretkey
		Rio_readinitb(&rio, connfd);
		Rio_readnb(&rio, buf, sizeof(int));

		// Convert clientkey from bytes to unsig int
		clientkey = ((buf[0] & 0xFF) << 24) | ((buf[1] & 0xFF) << 16) | 
			((buf[2] & 0xFF) << 8) | (buf[3] & 0xFF);
		printf("Secret Key: %u\n", clientkey);

		if (realkey != clientkey) {
			clientbuf[0] = (char)1;
			Rio_writen(connfd, clientbuf, 1);
			Close(connfd);
			printf("--------------------------\n");
			continue;
		}

		clientbuf[0] = (char) 0;
		Rio_writen(connfd, clientbuf, 1);

		// Get client program type
		Rio_readnb(&rio, buf, 1);
		type = buf[0] & 0xFF;
		printf("Type: %d\n", type);
		switch (type) {
			case 0:
				simpleSet("TEST", "test", 4);
				break;
			case 1:
				simpleGet("TEST", "GET");
				break;
			case 2:
				simpleDigest("DATA", 8, "RESULT");
				break;
			case 3:
				simpleRun("RUN", "RESULT");
				break;
			default:
				printf("Invalid request\n");
				break;
		}	
		// Close connection
		Close(connfd);
		printf("--------------------------\n");
	}

	free(varName);
	free(varValue);
	return 0;
}
