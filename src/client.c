#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "buffer.h"
#include "global_const.h"


extern int run;

static int sockfd;
static int portno;
static struct hostent* server;
static struct sockaddr_in serv_addr;

static IplImage* image;

/****************** network functions ******************/

// convert hostname into hostend type
void setHostname(char hostname[]) {
	server = gethostbyname(hostname);
	if(server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(EXIT_FAILURE);
	}

	printf("Server-Address is: %s\n", hostname);fflush(stdout);
}

// convert portnumber from char to int
void setPort(char port[]) {
	portno = atoi(port);
	if(port == 0) {
		fprintf(stderr,"ERROR, no such port\n");
		exit(EXIT_FAILURE);
	}

	printf("Server-Port is: %s\n", port);fflush(stdout);
}

// create new socket, socket()
static void createSocket() {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("ERROR opening socket");
		exit(EXIT_FAILURE);
	}

	printf("Client: socket created\n");fflush(stdout);
}

static void prepareConnect() {
	// fill struct serv_addr with zeros
	bzero((char*)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno); // for conversation in network byte order
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
}

// close connection, close()
static void closeConnection() {
	close(sockfd);

	printf("Client: socket closed\n");fflush(stdout);
}

// open connection, connect()
static void openConnection() {
	int res = -1;
	int i = 0;

	while((i != CONNECT_RETRIES-1) && res == -1) {
			res = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

			if(res != -1) {
				printf("Client: trying to connect to server, success!\n");fflush(stdout);
			} else {
				printf("Client: trying to connect to server, failed!\n");fflush(stdout);
				sleep(WAIT);
			}

			i++;
	}

	if(res == CONNECT_RETRIES-1) {
		closeConnection();
		printf("Client: unable to connect to server, exciting!\n");fflush(stdout);
		exit(EXIT_FAILURE);
	}
}

/****************** screen output functions ******************/

static void prepareScreenOutput() {
	image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
	image->imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE;

	cvNamedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
}

static void showImage() {
	cvShowImage(WINDOW_NAME, image);
	if((cvWaitKey(5) & 255) == 27) {
		return;
	}
}

void* client(void* arg) {
	int cBytes = 0;
	int res = 0;

	printf("Client: is running!\n");fflush(stdout);

	createSocket();
	prepareConnect();
	openConnection();
	prepareScreenOutput();

	while(run) {
		cBytes = image->imageSize;

		while(cBytes != 0) {
			res = read(sockfd, &image->imageData[image->imageSize - cBytes], cBytes);
			if(res <= 0){
				printf("Client: error reading from socket\n");fflush(stdout);
				closeConnection();
				createSocket();
				openConnection();

				cBytes = image->imageSize;
			}

			cBytes -= res;
		}

		write_Image(*image);
		showImage();
	}

	closeConnection();

	printf("Client: is exciting!\n");fflush(stdout);

	pthread_exit(NULL);
}
