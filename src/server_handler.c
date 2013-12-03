#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

#include "global_const.h"
#include "server.h"


extern int run;

static int socket_sockfd;
static int accept_sockfd;
static int portno;
static unsigned int client_len;
static struct sockaddr_in serv_addr;
static struct sockaddr_in cli_addr;

int num_servers; // amount of servers currently running
pthread_mutex_t num_servers_lock; // protects num_servers

/****************** network functions ******************/

void setPort_Server(char port[]) {
	portno = atoi(port);
	if(port == 0) {
		perror("Client-Handler: ERROR, no such port");
		exit(EXIT_FAILURE);
	}

	printf("Client-Handler: Servers reachable on port: %s\n", port);fflush(stdout);
}

// create new socket, socket()
static void createSocket() {
	int sock_opt = 1;

	socket_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_sockfd < 0) {
		perror("Server-Handler: ERROR, can't create socket");
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
		perror("Server-Handler: ERROR, can't bind");
		exit(EXIT_FAILURE);
	}

	printf("Server-Handler: socket bound\n");fflush(stdout);
}

// listening on a socket, listen()
static void listenOnSocket() {
	if(listen(socket_sockfd, 5) < 0) {
		perror("Server-Handler: ERROR, can't listen");
		exit(EXIT_FAILURE);
	}

	printf("Server-Handler: listening\n");fflush(stdout);
}

// close connection, close()
static void closeConnection() {
	close(socket_sockfd);

	printf("Server-Handler: socket closed\n");fflush(stdout);
}

/****************** threads ******************/

void* server_handler(void* arg) {
	int* new_sockfd;

	printf("Server-Handler: is running!\n");fflush(stdout);

	if(pthread_mutex_init(&num_servers_lock, NULL) != 0) {
		perror("Server-Handler: ERROR, failed to init mutex");
		exit(EXIT_FAILURE);
	}

	createSocket();
	prepareConnect();
	bindSocket();
	listenOnSocket();

	while(run) {
		// accept()
		client_len = sizeof(cli_addr);

		do {
			accept_sockfd = accept(socket_sockfd, (struct sockaddr*) &cli_addr, &client_len);
		} while((errno == EINTR) && accept_sockfd < 0);

		if(accept_sockfd < 0) {
			perror("Server-Handler: ERROR, during accept");
			exit(EXIT_FAILURE);
		}

		printf("Server-Handler: received incoming connection\n");fflush(stdout);

		/************* handling num_servers *************/

		if(pthread_mutex_lock(&num_servers_lock) != 0) {
			perror("Server-Handler: ERROR, can't lock mutex");
			exit(EXIT_FAILURE);
		}

		printf("Server-handler: current amount of running servers = %d, maximum = %d\n", num_servers, MAX_CLIENTS);fflush(stdout);

		if(num_servers < MAX_CLIENTS) {
			printf("Server-Handler: creating new server\n");fflush(stdout);

			new_sockfd = (int*)malloc(sizeof(accept_sockfd));
			*new_sockfd = accept_sockfd;

			// create new server thread
			pthread_t server_thread;
			pthread_create(&server_thread, NULL, &server, (void*)new_sockfd);

			num_servers++;

		} else {
			printf("Server-Handler: max amount of running servers already reached, closing connection\n");fflush(stdout);

			close(accept_sockfd);
		}

		if(pthread_mutex_unlock(&num_servers_lock) != 0) {
			perror("Server-Handler: ERROR, can't unlock mutex");
			exit(EXIT_FAILURE);
		}

		/************* done with handling num_servers *************/
	}

	closeConnection();

	if(pthread_mutex_destroy(&num_servers_lock) != 0) {
		perror("Server-Handler: ERROR, failed to init mutex");
		exit(EXIT_FAILURE);
	}

	printf("Client-Handler: is exiting!\n");fflush(stdout);

	pthread_exit(NULL);
}
