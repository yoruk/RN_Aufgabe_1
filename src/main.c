#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "buffer.h"
#include "client.h"
#include "server_handler.h"
#include "global_const.h"


int run = TRUE;

//static pthread_t client_thread;
//static pthread_t server_handler_thread;

int main(int argc, char *argv[]) {
//	if (argc < 4) {
//	   fprintf(stderr, "usage %s hostname port-client port-server\n", argv[0]);
//	   exit(EXIT_FAILURE);
//	}
//
//	setHostname(argv[1]);
//	setPort_Client(argv[2]);
//	setPort_Server(argv[3]);
//
//	pthread_create(&client_thread, NULL, &client, NULL);
//	pthread_create(&server_handler_thread, NULL, &server_handler, NULL);
//
////	sleep(RUNTIME);
////	run = FALSE;
//
//	pthread_join(server_handler_thread, NULL);
//	pthread_join(client_thread, NULL);


	// BUFFER DEBUG
	rawImage_t* input_image = (rawImage_t*)malloc(sizeof(rawImage_t));
	rawImage_t* output_image = (rawImage_t*)malloc(sizeof(rawImage_t));
	bufferEntry_t* entry = (bufferEntry_t*)malloc(sizeof(bufferEntry_t));
	int res;

	entry->last_oldest_image_idx = 0;
	entry->last_read_idx = -1;
	entry->offset = 0;

	printf("buffer test:\n\n");fflush(stdout);

	/************ buffer-test #1 ************/

	int i;
	for(i=0; i<4; i++) {
		printf("############# write %2d #############\n", i);fflush(stdout);

		input_image->data[0] = i;
		write_Image(input_image);

	}

	for(i=0; i<6; i++) {
		printf("############# read %2d #############\n", i);fflush(stdout);

		res = read_Image(output_image, entry);

		printf("res = %d\n", res);fflush(stdout);
		printf("data[0] = %d\n", output_image->data[0]);fflush(stdout);
	}

//	/************ buffer-test #2 ************/
//
//	int i;
//	for(i=0; i<=12; i++) {
//		printf("############# %2d #############\n", i);fflush(stdout);
//
//		input_image->data[0] = i;
//		write_Image(input_image);
//
//		res = read_Image(output_image, entry);
//
//		printf("res = %d\n", res);fflush(stdout);
//		printf("data[0] = %d\n", output_image->data[0]);fflush(stdout);
//	}

	return EXIT_SUCCESS;
}

