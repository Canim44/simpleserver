#include "csapp.h"
#define MAXVARVALUE 100
#define MAXVARNAME 16
void echo(int connfd) { printf("%d", connfd); }

char** varName;		// environment variable names
char** varValue;	// environment variable values
int varSize = 10;	// default size of var array (flexible)
int nVars = 0;		// index of last set variable value
unsigned int realkey;	// actual secret key value to access server

int simpleSet(char *variableName, char *value, int dataLength) {
	printf("Request type = set\n");

	// check if more space is needed; allocate more if so
	if (varSize <= nVars) {
		varSize += 10;
		varName = realloc(varName, varSize);
		varValue = realloc(varValue, varSize);
		if (!varName || !varValue) {
			fprintf(stderr, "allocation error\n");
			exit(EXIT_FAILURE);
		}

		int i;
		for (i = nVars; i < varSize; i++) {
			varName[i] = malloc(MAXVARNAME+1);
			varValue[i] = malloc(MAXVARVALUE+1);
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

int simpleGet(int connfd, char *variableName) {
	printf("Request type = get\n");

	int i, length, send_length;
	int index = -1;
	for (i = 0; i < nVars; i++) {
		if (strcmp(varName[i], variableName) == 0) {
			index = i;
			break;
		}
	}
	
	if (index != -1) {
		length = strlen(varValue[index]);
		send_length = htonl(length);
		Rio_writen(connfd, &send_length, sizeof(int));
		Rio_writen(connfd, varValue[index], length);
		printf("Detail = %s: %s\n", varName[index], varValue[index]);
		printf("Completion = success\n");
		return 0;
	}

	length = 0;
	Rio_writen(connfd, &length, sizeof(int));
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
	varName = malloc(varSize * MAXVARNAME);
	varValue = malloc(varSize * MAXVARVALUE);
	for (i = 0; i < varSize; i++) {
		varName[i] = (char*)malloc(MAXVARNAME+1);
		varValue[i] = (char*)malloc(MAXVARVALUE+1);
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
		memset(buf, '\0', sizeof(buf));
		Rio_readnb(&rio, buf, sizeof(int));

		// Convert clientkey from bytes to unsig int
		clientkey = ((buf[0] & 0xFF) << 24) | ((buf[1] & 0xFF) << 16) | 
			((buf[2] & 0xFF) << 8) | (buf[3] & 0xFF);
		printf("Secret Key: %u\n", clientkey);

		if (realkey != clientkey) {
			Close(connfd);
			printf("--------------------------\n");
			continue;
		}

		// Get client program type
		Rio_readnb(&rio, buf, 1);
		type = buf[0] & 0xFF;
		printf("Type: %d\n", type);
		// get 3 bytes of padding
		Rio_readnb(&rio, buf, 3);
		switch (type) {
			case 0: ;
				char *name = malloc(MAXVARNAME);
				char *value;
				int size;
				Rio_readnb(&rio, buf, 16);	// name of variable
				strncpy(name, buf, 16);
				for (int i = 0; i < 16; i++) {
					printf("0x%02x ", name[i]);
				}
				printf("\n");
				Rio_readnb(&rio, buf, sizeof(int));	// size of value
				size = ((buf[0] & 0xFF) << 24) | ((buf[1] & 0xFF) << 16) |
					((buf[2] & 0xFF) << 8) | (buf[3] & 0xFF);
				value = malloc(size);
				Rio_readnb(&rio, buf+16, size);	// value of variable
				strncpy(value, buf+16, size);
				simpleSet(name, value, size);

				free(name);
				free(value);
				break;

			case 1: ;
				char *vname = malloc(MAXVARNAME);
				Rio_readnb(&rio, buf, 16);
				strncpy(vname, buf, 16);
				simpleGet(connfd, vname);

				free(name);
				break;

			case 2: ;
				simpleDigest("DATA", 8, "RESULT");
				break;

			case 3: ;
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
