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

#include "global_const.h"
#include "buffer.h"

struct sender_args {
	imageBuffer_t* imageBuffer;
	int sockfd;
	sem_t* sender_wait;
	int* exit;
};

extern int run;

extern rawImage_t* input_image;
extern int done_reading;
extern pthread_mutex_t input_image_mutex;
extern pthread_cond_t input_image_cond;

extern int num_servers;
extern sem_t block_accept;

static int read_counter = 0; // takes care that all server did read the image

static void* sender(void* args) {
	struct sender_args* non_void_args = (struct sender_args*)args;
	rawImage_t* output_image;
	int cBytes = 0;
	int res = 0;

	printf("Sender: is running!\n");fflush(stdout);

	// malloc for data structures
	output_image = (rawImage_t*)malloc(sizeof(rawImage_t));

	while(run && !*non_void_args->exit) {
		// get image from buffer
		if(read_Image(non_void_args->imageBuffer, output_image) != 0) {
//			printf("Sender: buffer empty!\n");fflush(stdout);

			// wait, buffer is empty
			if(sem_wait(non_void_args->sender_wait) != 0) {
				perror("Sender: ERROR, failed to wait on semaphore");
				exit(EXIT_FAILURE);
			}

			// try again
			read_Image(non_void_args->imageBuffer, output_image);
		}

		// determine how many bytes to write
		cBytes = IMAGE_SIZE;

		// write
		while(cBytes != 0 && !*non_void_args->exit) {
			res = write(non_void_args->sockfd, &output_image->data[IMAGE_SIZE - cBytes], cBytes);

			if(res <= 0) {
				printf("Server: ERROR, can't write to socket, exciting!\n");fflush(stdout);

				//exit_server = TRUE;
				//exit_buffer_feeder =  TRUE;
				*non_void_args->exit = TRUE;

				break;
			}

			cBytes -= res;
		}
	}

	close(non_void_args->sockfd);

	free(output_image);

	printf("Sender: is exciting!\n");fflush(stdout);

	pthread_exit(NULL);
}

void* server(void* new_sockfd) {
	imageBuffer_t* imageBuffer = NULL;
	sem_t sender_wait; // let server wait if buffer is empty
	struct sender_args* args = NULL; // arguments for sender
	pthread_t sender_thread;
	int exit_server = FALSE;
	int exit_sender = FALSE;
	int first_time = TRUE;

	printf("Server: is running!\n");fflush(stdout);

	if(sem_init(&sender_wait, 0, 0) != 0) {
		perror("Server: ERROR, failed to init semaphore");
		exit(EXIT_FAILURE);
	}

	// malloc for data structures
	imageBuffer = (imageBuffer_t*)malloc(sizeof(imageBuffer_t));
	args = (struct sender_args*)malloc(sizeof(struct sender_args));

	// set up imageBuffer
	imageBuffer->write_idx = 0;
	imageBuffer->read_idx = 0;
	imageBuffer->count = 0;
	if(pthread_mutex_init(&imageBuffer->lock, NULL) != 0) {
		perror("Server: ERROR, failed to init mutex");
		exit(EXIT_FAILURE);
	}

	// start sender
	args->imageBuffer = imageBuffer;
	args->sockfd = *(int*)new_sockfd;
	args->sender_wait = &sender_wait;
	args->exit = &exit_sender;
	if(pthread_create(&sender_thread, NULL, &sender, (void*)args) !=0 ) {
		perror("Server: ERROR, could not start buffer feeder thread");
		exit(EXIT_FAILURE);
	}

	while(run && !exit_server) {
		/************* begin critical section *************/

		if(pthread_mutex_lock(&input_image_mutex) != 0) {
			perror("Server: ERROR, can't lock mutex");
			exit(EXIT_FAILURE);
		}

		while(done_reading == TRUE) {
			if(pthread_cond_wait( &input_image_cond, &input_image_mutex) != 0) {
				perror("Server: ERROR, can't wait for condition variable");
				exit(EXIT_FAILURE);
			}
		}

		// increase number of servers
		if(first_time) {
			num_servers++;
			first_time = FALSE;
		}

		// if MAX_CLIENTS is reached, dont free semaphore in server_handler
		if(num_servers < MAX_CLIENTS) {
			if(sem_post(&block_accept) != 0) {
				perror("Server: ERROR, failed to post on semaphore");
				exit(EXIT_FAILURE);
			}
		}

//		printf("DEBUG: server in critical section, num_servers=%d\n", num_servers);fflush(stdout);

		// write actual image into buffer
		write_Image(imageBuffer, input_image);

		// set done_reading flag if all server did read the image
		read_counter++;
		if(read_counter >= num_servers) {
			read_counter = 0;

			done_reading = TRUE;
		}

		// if sender has been canceled,
		// also cancel server and decrease num_servers
		if(exit_sender == TRUE) {
			exit_server = TRUE;

			num_servers--;

			if(sem_post(&block_accept) != 0) {
				perror("Server: ERROR, failed to post on semaphore");
				exit(EXIT_FAILURE);
			}
		}

		if(pthread_cond_signal(&input_image_cond) != 0) {
			perror("Server: ERROR, failed to broadcast on condition variable");
			exit(EXIT_FAILURE);
		}

		if(pthread_mutex_unlock(&input_image_mutex) != 0) {
			perror("Server: ERROR, can't unlock mutex");
			exit(EXIT_FAILURE);
		}

		/************* end critical section *************/

		if(sem_post(&sender_wait) != 0) {
			perror("Server: ERROR, failed to post on semaphore");
			exit(EXIT_FAILURE);
		}
	}

	pthread_join(sender_thread, NULL);

	free(imageBuffer);
	free(new_sockfd);
	free(args);

	printf("Server: is exciting!\n");fflush(stdout);

	pthread_exit(NULL);
}

