#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "global_const.h"
#include "buffer.h"

//typedef struct {
//	char data[IMAGE_SIZE];
//} rawImage_t;

//typedef struct {
//	rawImage_t data[BUFFER_SIZE];
//	unsigned int write_idx;
//	unsigned int read_idx;
//	unsigned int count;
//	pthread_mutex_t lock;
//} imageBuffer_t;

//void init_ImageBuffer(imageBuffer_t* imageBuffer) {
//	imageBuffer = (imageBuffer_t*)malloc(sizeof(imageBuffer_t));
//
//	imageBuffer->write_idx = 0;
//	imageBuffer->read_idx = 0;
//	imageBuffer->count = 1;
//
//	if(pthread_mutex_init(&imageBuffer->lock, NULL) != 0) {
//		perror("Buffer: init_ImageBuffer(): ERROR, failed to init mutex");
//		exit(EXIT_FAILURE);
//	}
//}

void write_Image(imageBuffer_t* imageBuffer, rawImage_t* image) {
	// lock
	if(pthread_mutex_lock(&imageBuffer->lock) != 0) {
		perror("Buffer: write_Image(): ERROR, failed to lock mutex");
		exit(EXIT_FAILURE);
	}

	// write into buffer
	imageBuffer->data[imageBuffer->write_idx] = *image;

	// move write index to next position
	imageBuffer->write_idx = (imageBuffer->write_idx + 1) % BUFFER_SIZE;

	// drop oldest image if necessary,
	// increase count if buffer isnt full already
	if(imageBuffer->count == BUFFER_SIZE) {
		imageBuffer->read_idx = (imageBuffer->read_idx + 1) % BUFFER_SIZE;
	} else {
		imageBuffer->count++;
	}

	// DEBUG
	if((imageBuffer->count < 0) || (imageBuffer->count > BUFFER_SIZE)) {
		printf("Buffer: DEBUG: write_Image() count = %d\n", imageBuffer->count);fflush(stdout);
	}

	// DEBUG
	//printf("write: count = %d\n", imageBuffer->count);fflush(stdout);

	// unlock
	if(pthread_mutex_unlock(&imageBuffer->lock) != 0) {
		perror("Buffer: write_Image(): ERROR, failed to unlock mutex");
		exit(EXIT_FAILURE);
	}
}

int read_Image(imageBuffer_t* imageBuffer, rawImage_t* image) {
	// lock
	if(pthread_mutex_lock(&imageBuffer->lock) != 0) {
		perror("Buffer: read_Image(): ERROR, failed to lock mutex");
		exit(EXIT_FAILURE);
	}

	// quit if buffer is empty
	if(imageBuffer->count == 0) {
		// unlock
		if(pthread_mutex_unlock(&imageBuffer->lock) != 0) {
			perror("Buffer: read_Image(): ERROR, failed to unlock mutex");
			exit(EXIT_FAILURE);
		}

		return EXIT_FAILURE;
	}

	// read from buffer
	*image = imageBuffer->data[imageBuffer->read_idx];

	// move read index to next position
	imageBuffer->read_idx = (imageBuffer->read_idx + 1) % BUFFER_SIZE;

	// decrease count
	imageBuffer->count--;

	// DEBUG
	if((imageBuffer->count < 0) || (imageBuffer->count > BUFFER_SIZE)) {
		printf("Buffer: DEBUG: read_Image() count = %d\n", imageBuffer->count);fflush(stdout);
	}

	// DEBUG
	//printf("read: count = %d\n", imageBuffer->count);fflush(stdout);

	// unlock
	if(pthread_mutex_unlock(&imageBuffer->lock) != 0) {
		perror("Buffer: write_Image(): ERROR, failed to unlock mutex");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

