#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

#include "global_const.h"
#include "buffer.h"

struct buffer_feeder_args {
	imageBuffer_t* imageBuffer;
	int* exit;
};

extern int run;

extern int num_accesses;
extern int num_servers;
extern pthread_mutex_t server_admin_mutex;

extern rawImage_t* input_image;
extern pthread_mutex_t input_image_mutex;
extern pthread_cond_t input_image_cond_client;
extern pthread_cond_t input_image_cond_servers;

static void* buffer_feeder(void* args) {
	struct buffer_feeder_args* non_void_args = (struct buffer_feeder_args*)args;

	while(!non_void_args->exit) {
		if(pthread_mutex_lock(&input_image_mutex) != 0) {
			perror("Server: data_provider(): ERROR, can't lock mutex");
			exit(EXIT_FAILURE);
		}

		if(pthread_cond_wait( &input_image_cond_servers, &input_image_mutex) != 0) {
			perror("Server: data_provider(): ERROR, can't wait for condition variable");
			exit(EXIT_FAILURE);
		}

		write_Image(non_void_args->imageBuffer, input_image);
		num_accesses--;

		if(pthread_cond_signal(&input_image_cond_client) != 0) {
			perror("Server: data_provider(): ERROR, failed to broadcast on condition variable");
			exit(EXIT_FAILURE);
		}

		if(pthread_mutex_unlock(&input_image_mutex) != 0) {
			perror("Server: data_provider(): ERROR, can't unlock mutex");
			exit(EXIT_FAILURE);
		}
	}

	pthread_exit(NULL);
}

void* server(void* new_sockfd) {
	imageBuffer_t* imageBuffer = NULL;
	rawImage_t* output_image = NULL;
	int cBytes = 0;
	int res = 0;
	int retries = 0;
	int exit_server = FALSE;
	int exit_buffer_feeder = FALSE;
	struct buffer_feeder_args* args = NULL;
	pthread_t buffer_feeder_thread;

	printf("Server: is running!\n");fflush(stdout);

	// malloc for data structures
	//init_ImageBuffer(imageBuffer);
	imageBuffer = (imageBuffer_t*)malloc(sizeof(imageBuffer_t));
	output_image = (rawImage_t*)malloc(sizeof(rawImage_t));
	args = (struct buffer_feeder_args*)malloc(sizeof(struct buffer_feeder_args));

	// set up imageBuffer
	imageBuffer->write_idx = 0;
	imageBuffer->read_idx = 0;
	imageBuffer->count = 1;

	if(pthread_mutex_init(&imageBuffer->lock, NULL) != 0) {
		perror("Buffer: init_ImageBuffer(): ERROR, failed to init mutex");
		exit(EXIT_FAILURE);
	}

	// start data provider
	args->exit = &exit_buffer_feeder;
	args->imageBuffer = imageBuffer;
	if(pthread_create(&buffer_feeder_thread, NULL, &buffer_feeder, (void*)args) !=0 ) {
		perror("Server: ERROR, could not start buffer feeder thread");
		exit(EXIT_FAILURE);
	}

	while(run && !exit_server) {
		// get image from buffer
		if(read_Image(imageBuffer, output_image) != 0){

			if(retries == BUFFER_READ_RETRIES) {
				printf("Server: buffer empty, closing connection!\n");fflush(stdout);

				exit_server = TRUE;
				exit_buffer_feeder =  TRUE;
			} else {
				printf("Server: ERROR, buffer empty\n");fflush(stdout);

				retries++;

				sleep(WAIT_SHORT);
			}


		} else {
			// reset retry counter
			retries = 0;

			// determine how many bytes to write
			cBytes = IMAGE_SIZE;

			// write
			while(cBytes != 0 && !exit_server) {
				res = write(*(int*)new_sockfd, &output_image->data[IMAGE_SIZE - cBytes], cBytes);
				if(res <= 0){
					printf("Server: ERROR, can't write to socket, exciting!\n");fflush(stdout);

					exit_server = TRUE;
					exit_buffer_feeder =  TRUE;

					break;
				}

				cBytes -= res;
			}
		}
	}

	pthread_join(buffer_feeder_thread, NULL);

	close(*(int*)new_sockfd);
	free(new_sockfd);

	free(imageBuffer);
	free(output_image);
	free(args);

	num_servers--;

	printf("Server: is exciting!\n");fflush(stdout);

	pthread_exit(NULL);
}
