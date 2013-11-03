#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "client.h"
#include "dummy_server.h"
#include "global_const.h"

int run = TRUE;
static pthread_t t1;
//static pthread_t t2;

int main(int argc, char *argv[]) {
	if (argc < 3) {
	   fprintf(stderr,"usage %s hostname port\n", argv[0]);
	   exit(EXIT_FAILURE);
	}

	setHostname(argv[1]);
	setPort(argv[2]);

	pthread_create(&t1, NULL, &client, NULL);
//	pthread_create(&t2, NULL, &dummy_server, NULL);

	sleep(RUNTIME);
	run = FALSE;

	pthread_join(t1, NULL);
//	pthread_join(t2, NULL);

	return EXIT_SUCCESS;
}

