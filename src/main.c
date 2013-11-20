#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "client.h"
//#include "server_handler.h"
#include "global_const.h"

int run = TRUE;

static pthread_t client_thread;
//static pthread_t server_handler_thread;

int main(int argc, char *argv[]) {
	if (argc < 4) {
	   fprintf(stderr, "usage %s hostname port-client port-server\n", argv[0]);
	   exit(EXIT_FAILURE);
	}

	setHostname(argv[1]);
	setPort_Client(argv[2]);
//	setPort_Server(argv[3]);

	pthread_create(&client_thread, NULL, &client, NULL);
//	pthread_create(&server_handler_thread, NULL, &server_handler, NULL);

//	sleep(RUNTIME);
//	run = FALSE;

//	pthread_join(server_handler_thread, NULL);
	pthread_join(client_thread, NULL);

	return EXIT_SUCCESS;
}

