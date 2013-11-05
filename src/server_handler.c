#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

#include "buffer.h"
#include "global_const.h"


extern int run;

static int socket_sockfd;
static int accept_sockfd;
static int portno;
static unsigned int client_len;
static struct sockaddr_in serv_addr;
static struct sockaddr_in cli_addr;

static IplImage* output_image;
static pthread_t data_provider_thread;

static int numClients = 0;
static int sock_opt = 1;

void setPort_Server(char port[]) {
	portno = atoi(port);
	if(port == 0) {
		fprintf(stderr,"ERROR, no such port\n");
		exit(EXIT_FAILURE);
	}

	printf("Server running on port: %s\n", port);fflush(stdout);
}

static void* data_provider(void* arg) {
	while(run) {
		if(numClients != 0) {
			output_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
			output_image->imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE;
			read_Image(output_image);
		}

		usleep(DATA_PROVIDER_WAIT);
	}

	pthread_exit(NULL);
}

void* server_handler(void* arg) {
	IplImage* buffered_output_image;
	int cBytes;
	int res;

	// debug
	output_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
	output_image->imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE;

	// start data provider
	pthread_create(&data_provider_thread, NULL, &data_provider, NULL);

	// socket()
	socket_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_sockfd < 0)
	  printf("ERROR opening socket\n");fflush(stdout);
	setsockopt(socket_sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));


	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// bind()
	if (bind(socket_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	  printf("ERROR on binding\n");fflush(stdout);

	// listen()
	listen(socket_sockfd,5);

	// accept()
	client_len = sizeof(cli_addr);
	accept_sockfd = accept(socket_sockfd, (struct sockaddr *) &cli_addr, &client_len);
	if (accept_sockfd < 0)
	  printf("ERROR on accept");fflush(stdout);

	printf("Server: Client connected\n");fflush(stdout);

	numClients++;

	buffered_output_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
	buffered_output_image->imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE;


	while(run) {
		cBytes = buffered_output_image->imageSize;
		bcopy(output_image->imageData, buffered_output_image->imageData, buffered_output_image->imageSize);

		while(cBytes != 0) {
			res = write(accept_sockfd, &buffered_output_image->imageData[buffered_output_image->imageSize - cBytes], cBytes);
			if(res <= 0){
				printf("Server: error writing to socket\n");fflush(stdout);

				sleep(1);

				cBytes = buffered_output_image->imageSize;
			}

			cBytes -= res;
		}
	}


	close(accept_sockfd);
	close(socket_sockfd);

	pthread_exit(NULL);
}
