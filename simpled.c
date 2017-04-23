#include "csapp.h"
#define MAXVARVALUE 100
#define MAXVARNAME 16
void echo(int connfd) { printf("%d", connfd); }

char** varName;		// environment variable names
char** varValue;	// environment variable values
char* junk = "dog"; 	// 3 bytes of padding
int varSize = 512;	// default size of var array (flexible)
int nVars = 0;		// index of last set variable value
unsigned int realkey;	// actual secret key value to access server
char success = (char) 0;
char failure = (char) -1;

int simpleSet(int connfd, char *variableName, char *value, int dataLength) {
	printf("Request type = set\n");

	// check if more space is needed; allocate more if so
	if (varSize <= nVars) {
		varSize += 512;
		varName = realloc(varName, varSize);
		varValue = realloc(varValue, varSize);
		if (!varName || !varValue) {
			fprintf(stderr, "allocation error\n");
			exit(EXIT_FAILURE);
		}

		int i;
		for (i = nVars; i < varSize; i++) {
			varName[i] = malloc(MAXVARNAME+1);
			varValue[i] = malloc(MAXVARVALUE);
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

	Rio_writen(connfd, &success, sizeof(char));
	Rio_writen(connfd, junk, 3*sizeof(char));
	printf("Detail = %s: %s\n", varName[index], varValue[index]);
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
		Rio_writen(connfd, &success, sizeof(char));
		Rio_writen(connfd, junk, 3*sizeof(char));
		length = strlen(varValue[index]);
		send_length = htonl(length);
		Rio_writen(connfd, &send_length, sizeof(int));
		Rio_writen(connfd, varValue[index], length);
		printf("Detail = %s: %s\n", varName[index], varValue[index]);
		printf("Completion = success\n");
		return 0;
	}

	Rio_writen(connfd, &failure, sizeof(char));
	Rio_writen(connfd, junk, 3*sizeof(char));
	printf("Could not locate variable %s\n", variableName);
	return -2;
}

int simpleDigest(int connfd, char *data) {
	printf("Request type = digest\n");
	char command[256] = "sh -c \'echo `/bin/hostname` ";
	printf("assigned command: %s\n", command);
	printf("data: %s\n", data);
	strcat(command, data);
	printf("done with strcat: %s\n", command); 
	strcat(command, " | /usr/bin/md5sum\'");
	printf("done with command: %s\n", command);
	FILE* fp;
	char path[256];

	fp = popen(command, "r");
	if (fp == NULL) {
		Rio_writen(connfd, &failure, sizeof(char));
		Rio_writen(connfd, junk, 3*sizeof(char));
	}
	Rio_writen(connfd, &success, sizeof(char));
	Rio_writen(connfd, junk, 3*sizeof(char));
	printf("start whie loop\n");
	while (fgets(path, sizeof(path), fp) != NULL) {
		Rio_writen(connfd, path, 256);
	}
	printf("done with while loop\n");
	path[0] = '\0';
	Rio_writen(connfd, path, 256);
	pclose(fp);

	printf("Detail = %s\n", data);
	printf("Completion = success\n");
	return 0;
}

int simpleRun(int connfd, char *request) {
	printf("Request type = run\n");

	int byte_limit_reached = 0;
	int nBytes = 0;
	int nBytes_prev = 0;
	int send_length = 0;
	FILE* fp;
	char path[100];
	if (strcmp(request, "inet") == 0) {
		fp = popen("/sbin/ifconfig -a", "r");
	}
	else if (strcmp(request, "hosts") == 0) {
		fp = popen("/bin/cat /etc/hosts", "r");
	}
	else if (strcmp(request, "uptime") == 0) {
		fp = popen("/usr/bin/uptime", "r");
	}
	else {
		printf("Command not found\n");
		Rio_writen(connfd, &failure, sizeof(char));
		Rio_writen(connfd, junk, 3*sizeof(char));
		return -2;
	}
	
	Rio_writen(connfd, &success, sizeof(char));
	Rio_writen(connfd, junk, 3*sizeof(char));
	while (fgets(path, sizeof(path), fp) != NULL) {
		nBytes += strlen(path);
		if (nBytes > 100) {
			nBytes = 100;
		}
		send_length = htonl(nBytes - nBytes_prev);
		Rio_writen(connfd, &send_length, sizeof(int));
		
		if ((nBytes - nBytes_prev) == 0) {
			byte_limit_reached = 1;
			break;
		}

		Rio_writen(connfd, path, nBytes - nBytes_prev);
		nBytes_prev = nBytes;
	}
	
	if (byte_limit_reached == 0) {
		send_length = htonl(0);
		Rio_writen(connfd, &send_length, sizeof(int));
	}

	pclose(fp);

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
	varName = malloc(varSize);
	varValue = malloc(varSize);
	for (i = 0; i < varSize; i++) {
		varName[i] = (char*)malloc(MAXVARNAME+1);
		varValue[i] = (char*)malloc(MAXVARVALUE);
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
		Rio_readnb(&rio, buf+4, 1);
		type = buf[4] & 0xFF;
		// get 3 bytes of padding
		Rio_readnb(&rio, buf+5, 3);
		switch (type) {
			case 0: ;	// set
				char *name = malloc(MAXVARNAME);
				char *value;
				unsigned int size;
				Rio_readnb(&rio, buf+8, 16);	// name of variable
				strncpy(name, buf+8, 16);
				Rio_readnb(&rio, buf+24, sizeof(int));	// size of value
				size = ((buf[24] & 0xFF) << 24) | ((buf[25] & 0xFF) << 16) |
					((buf[26] & 0xFF) << 8) | (buf[27] & 0xFF);
				if (size > MAXVARVALUE) {
					size = MAXVARVALUE;
				}

				value = malloc(size);
				Rio_readnb(&rio, buf+28, size);	// value of variable
				strncpy(value, buf+28, size);
				simpleSet(connfd, name, value, size);

				free(name);
				free(value);
				break;

			case 1: ;	// get
				char *vname = malloc(MAXVARNAME);
				Rio_readnb(&rio, buf+8, 16);
				strncpy(vname, buf+8, 16);
				simpleGet(connfd, vname);

				free(vname);
				break;

			case 2: ;	// digest
				unsigned int digSize;
				char *digVal;
				Rio_readnb(&rio, buf+8, sizeof(int));	// size of value
				digSize = ((buf[8] & 0xFF) << 24) | ((buf[9] & 0xFF) << 16) |
					((buf[10] & 0xFF) << 8) | (buf[11] & 0xFF);
				if (digSize > MAXVARVALUE) {
					digSize = MAXVARVALUE;
				}

				digVal = malloc(digSize);
				Rio_readnb(&rio, buf+12, digSize);	// value of variable
				strncpy(digVal, buf+12, digSize);
				simpleDigest(connfd, digVal);

				free(digVal);
				break;

			case 3: ;	// run
				char* command = malloc(8);
				Rio_readnb(&rio, buf+8, 8);
				strncpy(command, buf+8, 8);
				simpleRun(connfd, command);

				free(command);
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
