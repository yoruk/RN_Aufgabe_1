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
#include <opencv/highgui.h>
#include "global_const.h"

static IplImage* image;

int main(int argc, char *argv[]) {
	int portno;
	struct hostent *server;
	int sockfd;
	struct sockaddr_in serv_addr;
	int i;
	int n;
	int cBytes;

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

	image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 3);
	image->imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE;

	cvNamedWindow("Simulator", CV_WINDOW_AUTOSIZE);

	while(1) {
		cBytes = image->imageSize;
		printf("cBytes vor while: %d\n", cBytes);fflush(stdout);

		while(cBytes != 0) {
			n = read(sockfd, &image->imageData[image->imageSize - cBytes], cBytes);
			if(n <= 0){
				printf("ERROR reading from socket, closing connection\n");
				close(sockfd);
				return EXIT_SUCCESS;
			}

			cBytes -= n;

		}

		printf("cBytes nach while: %d\n\n", cBytes);fflush(stdout);

		cvShowImage("Simulator", image);
		if ((cvWaitKey(5) & 255) == 27) {
			break;
		}

		//sleep(2);
	}

	// close
	close(sockfd);

	return EXIT_SUCCESS;
}

