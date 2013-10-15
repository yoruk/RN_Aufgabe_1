#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <opencv/cv.h>
#include "global_const.h"

int main(int argc, char *argv[]) {
	int sockfd;
	int portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	if (argc < 3) {
	   fprintf(stderr,"usage %s hostname port\n", argv[0]);
	   exit(EXIT_FAILURE);
	}

	// portnummer von char nach int konvertieren
	portno = atoi(argv[2]);

	// neuen socket erstellen
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(EXIT_FAILURE);
	}

	// hostname in hostend typ umwandeln
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	// struct serv_addr mit nullen fuellen
	bzero((char*)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno); // htons() fuer konvertierung in network byte order

	// connect
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR connecting");
		exit(EXIT_FAILURE);
	}

	printf("Connect succesful!\n");

	// close
	close(sockfd);

	return EXIT_SUCCESS;
}

