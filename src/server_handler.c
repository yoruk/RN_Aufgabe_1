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

void setPort_Server(char port[]) {
	portno = atoi(port);
	if(port == 0) {
		fprintf(stderr,"ERROR, no such port\n");
		exit(EXIT_FAILURE);
	}

	printf("Server running on port: %s\n", port);fflush(stdout);
}

/****************** network functions ******************/

// create new socket, socket()
static void createSocket() {
	int sock_opt = 1;

	socket_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_sockfd < 0) {
		fprintf(stderr,"ERROR opening socket");
		exit(EXIT_FAILURE);
	}

	setsockopt(socket_sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));

	printf("Server-Handler: socket created\n");fflush(stdout);
}

static void prepareConnect() {
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
}

// binding interfaces, bind()
static void bindSocket() {
	if(bind(socket_sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr,"ERROR opening socket");
		exit(EXIT_FAILURE);
	}

	printf("Server-Handler: socket bound\n");fflush(stdout);
}

// listening on a socket, listen()
static void listenOnSocket() {
	listen(socket_sockfd,5);

	printf("Server-Handler: listening\n");fflush(stdout);
}

// close connection, close()
static void closeConnection() {
	close(socket_sockfd);

	printf("Server-Handler: socket closed\n");fflush(stdout);
}

/****************** threads ******************/

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

static void* server(void* new_sockfd) {
	IplImage* buffered_output_image;
	int cBytes;
	int res;
	int exit = FALSE;

	buffered_output_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
	buffered_output_image->imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE;

	while(run && !exit) {
		cBytes = buffered_output_image->imageSize;
		bcopy(output_image->imageData, buffered_output_image->imageData, buffered_output_image->imageSize);

		while(cBytes != 0 && !exit) {
			res = write(*(int*)new_sockfd, &buffered_output_image->imageData[buffered_output_image->imageSize - cBytes], cBytes);
			if(res <= 0){
				printf("Server: error writing to socket, exciting!\n");fflush(stdout);

				//cBytes = buffered_output_image->imageSize;

				exit = TRUE;
				break;
			}

			cBytes -= res;
		}
	}

	free(new_sockfd);
	close(*(int*)new_sockfd);

	numClients--;

	pthread_exit(NULL);
}

void* server_handler(void* arg) {
	int* new_sockfd;

	printf("Server-Handler: is running!\n");fflush(stdout);

	// debug
	output_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
	output_image->imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE;

	// start data provider
	pthread_create(&data_provider_thread, NULL, &data_provider, NULL);

	createSocket();
	prepareConnect();
	bindSocket();
	listenOnSocket();

	while(run) {
		// accept()
		client_len = sizeof(cli_addr);
		accept_sockfd = accept(socket_sockfd, (struct sockaddr *) &cli_addr, &client_len);

		if(numClients <= MAX_CLIENTS) {

			printf("Server: Client connected\n");fflush(stdout);

			numClients++;

			new_sockfd = malloc(sizeof(accept_sockfd));
			*new_sockfd = accept_sockfd;

			pthread_t server_type;
			pthread_create(&server_type, NULL, &server, (void*)new_sockfd);
		}
	}

	closeConnection();

	printf("Client-Handler: is exciting!\n");fflush(stdout);

	pthread_exit(NULL);
}
