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


extern int run;

extern int num_servers;
extern pthread_mutex_t num_servers_lock;

void* server(void* new_sockfd) {
	rawImage_t* output_image;
	bufferEntry_t* buffer_entry;
	int exit_server = FALSE;
	int cBytes = 0;
	int res = 0;

	printf("Server: is running!\n");fflush(stdout);

	// malloc for data structures
	output_image = (rawImage_t*)malloc(sizeof(rawImage_t));
	buffer_entry =  (bufferEntry_t*)malloc(sizeof(bufferEntry_t));

	// init buffer_entry
	buffer_entry->last_oldest_image_idx = 0;
	buffer_entry->offset = 0;
	buffer_entry->first_run = TRUE;

	while(run && !exit_server) {
		// get image from buffer
		do {
			res = read_Image(output_image, buffer_entry);

			if(res != 0) {
				//printf("Server: buffer empty!\n");fflush(stdout);

				usleep(5000);
			}
		} while(res != 0);

		// determine how many bytes to write
		cBytes = IMAGE_SIZE;

		// write
		while(cBytes != 0 && !exit_server) {
			res = write(*(int*)new_sockfd, &output_image->data[IMAGE_SIZE - cBytes], cBytes);

			if(res <= 0) {
				printf("Server: ERROR, can't write to socket, exciting!\n");fflush(stdout);

				exit_server = TRUE;

				break;
			}

			cBytes -= res;
		}
	}

	close(*(int*)new_sockfd);

	free(new_sockfd);
	free(output_image);
	free(buffer_entry);

	/************* decreasing num_servers *************/

	if(pthread_mutex_lock(&num_servers_lock) != 0) {
		perror("Server: ERROR, can't lock mutex");
		exit(EXIT_FAILURE);
	}

	num_servers--;

	if(pthread_mutex_unlock(&num_servers_lock) != 0) {
		perror("Server: ERROR, can't unlock mutex");
		exit(EXIT_FAILURE);
	}

	/************* done with decreasing num_servers *************/

	printf("Server: is exciting!\n");fflush(stdout);

	pthread_exit(NULL);
}

