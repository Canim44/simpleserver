#include "csapp.h"
#define MAXVARLEN 128

void echo(int connfd) { printf("%d", connfd); }

char** varName;		// environment variable names
char** varValue;	// environment variable values
int varSize = 10;	// default size of var array (flexible)
int nVars = 0;		// index of last set variable value
unsigned int realkey;	// actual secret key value to access server

int simpleSet(char *MachineName, int TCPport, int SecretKey, char *variableName, char *value, int dataLength) {
	printf("Secret key = %d\n", SecretKey);
	if (SecretKey != realkey) {
		return -1;
	}
	
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

	printf("Detail = %s:%s\n", variableName, value);
	printf("Completion = success\n");
	return 0;
}

int simpleGet(char *MachineName, int TCPport, int SecretKey, char *variableName, char *value, int *resultLength) {
	printf("Secret key = %d\n", SecretKey);
	if (SecretKey != realkey) {
		return -1;
	}
	
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

int simpleDigest(char *MachineName, int TCPport, int SecretKey, char *data, int dataLength, char *result, int *resultLength) {
	printf("Secret key = %d\n", SecretKey);
	if (SecretKey != realkey) {
		return -1;
	}
	
	printf("Request type = digest\n");

	char* command = strcat("sh -c \'echo `/bin/hostname` ", data); 
	command = strcat(command, " | /usr/bin/md5sum");
	system(command);

	printf("Detail = %s\n", data);
	printf("Completion = success\n");
	return 0;
}

int simpleRun(char *MachineName, int TCPport, int SecretKey, char *request, char *result, int *resultLength) {
	printf("Secret key = %d\n", SecretKey);
	if (SecretKey != realkey) {
		return -1;
	}

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
	int listenfd, connfd, port;
	socklen_t clientlen;
	struct sockaddr_in clientaddr;
	struct hostent *hp;
	char *haddrp;
	char buf[MAXLINE];
	rio_t rio;
	if (argc != 3) {
		fprintf(stderr, "usage: %s <port> <secretKey>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[1]);
	realkey = atoi(argv[2]);

	//TODO: input handling for `port` and `secretkey`

	// allocate environment variables
	varName = malloc(varSize * (MAXVARLEN+1));
	varValue = malloc(varSize * (MAXVARLEN+1));

	listenfd = Open_listenfd(port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

		/* Determine the domain name and IP address of the client */
		hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		haddrp = inet_ntoa(clientaddr.sin_addr);
		printf("server connected to %s (%s)\n", hp->h_name, haddrp);
		
		Rio_readinitb(&rio, connfd);
		Rio_readnb(&rio, buf, sizeof(int));
		printf("Secret Key: 0x%02x 0x%02x 0x%02x 0x%02x\n", buf[0], buf[1], buf[2], buf[3]);
		Close(connfd);
	}

	free(varName);
	free(varValue);
	return 0;
}
