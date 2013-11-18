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

rawImage_t* input_image = NULL;
int done_reading = FALSE;
pthread_mutex_t input_image_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t input_image_cond = PTHREAD_COND_INITIALIZER;

extern int num_servers;

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

			if(res != -1) {
				printf("Client: trying to connect to server, success!\n");fflush(stdout);
			} else {
				printf("Client: trying to connect to server, failed!\n");fflush(stdout);
				sleep(WAIT);
			}

			i++;
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

/****************** other functions ******************/

static void cleanup() {
	if(pthread_cond_destroy(&input_image_cond) != 0) {
		perror("Client: ERROR, failed to destroy condition variable");
		exit(EXIT_FAILURE);
	}

//	if(pthread_cond_destroy(&input_image_cond_servers) != 0) {
//		perror("Client: ERROR, failed to destroy condition variable");
//		exit(EXIT_FAILURE);
//	}

	if(pthread_mutex_destroy(&input_image_mutex) != 0) {
		perror("Client: ERROR, failed to destroy mutex");
		exit(EXIT_FAILURE);
	}
}

/****************** thread functions ******************/

void* client(void* arg) {
	IplImage* tmp_image = NULL;
	int cBytes = 0;
	int res = 0;

	printf("Client: is running!\n");fflush(stdout);

	createSocket();
	prepareConnect();
	openConnection();
	//prepareScreenOutput(tmp_image);

	input_image = (rawImage_t*)malloc(sizeof(rawImage_t));

	tmp_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
	cvNamedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);

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

		/************* begin critical section *************/

		if(pthread_mutex_lock(&input_image_mutex) != 0) {
			perror("Client: ERROR, failed to lock mutex");
			exit(EXIT_FAILURE);
		}

		// if servers exist,
		// all servers need to be done with reading the last image
		while(num_servers != 0 && done_reading == FALSE) {
			if(pthread_cond_wait(&input_image_cond, &input_image_mutex) != 0) {
				perror("Client: ERROR, can't wait for condition variable");
				exit(EXIT_FAILURE);
			}
		}

//		printf("DEBUG: client in critical section, num_servers=%d\n", num_servers);fflush(stdout);

		bcopy((char*)tmp_image->imageData, (char*)input_image->data, IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE);

		done_reading = FALSE;

		if(pthread_cond_broadcast(&input_image_cond) != 0) {
			perror("Client: ERROR, failed to broadcast on condition variable");
			exit(EXIT_FAILURE);
		}

		if(pthread_mutex_unlock(&input_image_mutex) != 0) {
			perror("Client: ERROR, failed to unlock mutex\n");
			exit(EXIT_FAILURE);
		}

		/************* end critical section *************/

		showImage(tmp_image);
	}

	closeConnection();
	cleanup();

	printf("Client: is exciting!\n");fflush(stdout);

	pthread_exit(NULL);
}
