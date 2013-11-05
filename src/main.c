#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "client.h"
#include "server_handler.h"
#include "global_const.h"

int run = TRUE;
static pthread_t t1;
static pthread_t t2;

int main(int argc, char *argv[]) {
	if (argc < 4) {
	   fprintf(stderr,"usage %s hostname port-client port-server\n", argv[0]);
	   exit(EXIT_FAILURE);
	}

	setHostname(argv[1]);
	setPort_Client(argv[2]);
	setPort_Server(argv[3]);

	pthread_create(&t1, NULL, &client, NULL);
	pthread_create(&t2, NULL, &server_handler, NULL);

	sleep(RUNTIME);
	run = FALSE;

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return EXIT_SUCCESS;
}

