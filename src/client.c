#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

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

/****************** network functions ******************/

// convert hostname into hostend type
void setHostname(char hostname[]) {
	server = gethostbyname(hostname);
	if(server == NULL) {
		perror("Client: ERROR, no such host");
		exit(EXIT_FAILURE);
	}

	printf("Client: accessing Server: %s\n", hostname);fflush(stdout);
}

// convert portnumber from char to int
void setPort_Client(char port[]) {
	portno = atoi(port);
	if(port == 0) {
		perror("Client: ERROR, no such port");
		exit(EXIT_FAILURE);
	}

	printf("Client: accessing port: %s\n", port);fflush(stdout);
}

// create new socket, socket()
static void createSocket() {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("Client: ERROR, can't create socket");
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

			if(res == 0) {
				printf("Client: trying to connect to server, success!\n");fflush(stdout);
			} else {
				printf("Client: trying to connect to server, failed!\n");fflush(stdout);
				//sleep(WAIT);
			}

			i++;

			//sleep(WAIT);
			sleep(WAIT_SHORT);
	}

	if(i == CONNECT_RETRIES-1) {
		closeConnection();
		perror("Client: unable to connect to server, exciting!\n");fflush(stdout);
		exit(EXIT_FAILURE);
	}
}

/****************** screen output functions ******************/

//static void prepareScreenOutput(IplImage* image) {
//	image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
//	if(image == NULL) {
//		perror("Client: ERROR, can't create image");fflush(stdout);
//		exit(EXIT_FAILURE);
//	}
//
//	cvNamedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
//}

static void showImage(IplImage* image) {
	cvShowImage(WINDOW_NAME, image);
	if((cvWaitKey(5) & 255) == 27) {
		return;
	}
}

/****************** thread functions ******************/

void* client(void* arg) {
	IplImage* tmp_image = NULL;
	rawImage_t input_image;
	int cBytes = 0;
	int res = 0;

	printf("Client: is running!\n");fflush(stdout);

	createSocket();
	prepareConnect();
	openConnection();
	//prepareScreenOutput(tmp_image);

	tmp_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
	cvNamedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);

//	// DEBUG
//	rawImage_t* output_image;
//	output_image = (rawImage_t*)malloc(sizeof(rawImage_t));
//	bufferEntry_t* buffer_entry;
//	buffer_entry =  (bufferEntry_t*)malloc(sizeof(bufferEntry_t));
//	buffer_entry->last_oldest_image_idx = 0;
//	buffer_entry->offset = 0;

	while(run) {
		cBytes = tmp_image->imageSize;

		while(cBytes != 0) {
			res = read(sockfd, &tmp_image->imageData[tmp_image->imageSize - cBytes], cBytes);

			if(res <= 0){
				perror("Client: ERROR, can't read from socket");fflush(stdout);

				closeConnection();
				createSocket();
				openConnection();

				cBytes = tmp_image->imageSize;
			}

			cBytes -= res;
		}

		bcopy((char*)tmp_image->imageData, &input_image.data, IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE);
		write_Image(&input_image);

		// DEBUG
//		read_Image(output_image, buffer_entry);
//		bcopy((char*)output_image->data, (char*)tmp_image->imageData, IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE);

		showImage(tmp_image);

		//usleep(100000);
		//sleep(1);
	}

	closeConnection();

	printf("Client: is exiting!\n");fflush(stdout);

	pthread_exit(NULL);
}
