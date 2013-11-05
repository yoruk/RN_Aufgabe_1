#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "global_const.h"
#include "buffer.h"

typedef struct {
	IplImage data[BUFFER_SIZE];
	unsigned int write_idx;
	unsigned int read_idx;
	unsigned int count;
	pthread_mutex_t lock;
} imageBuffer_t;

static imageBuffer_t imageBuffer;
static int done = FALSE;

static void init_ImageBuffer() {
	imageBuffer.write_idx = 0;
	imageBuffer.read_idx = 0;
	imageBuffer.count = 0;

	if(pthread_mutex_init(&imageBuffer.lock, NULL) != 0) {
		perror("init_ImageBuffer(): failed to init mutex\n");
		exit(EXIT_FAILURE);
	}
}

void write_Image(IplImage image) {
	// lock
	if(pthread_mutex_lock(&imageBuffer.lock) != 0) {
		perror("write_Image(): failed to lock mutex\n");
		exit(EXIT_FAILURE);
	}

	// init buffer if necessary
	if(!done) {
		init_ImageBuffer();
		done = TRUE;
	}

	// write into buffer
	imageBuffer.data[imageBuffer.write_idx] = image;

	// move write index to next position
	imageBuffer.write_idx = (imageBuffer.write_idx + 1) % BUFFER_SIZE;

	// drop oldest image if necessary,
	// increase count if buffer isnt full already
	if(imageBuffer.count == BUFFER_SIZE) {
		imageBuffer.read_idx = (imageBuffer.read_idx + 1) % BUFFER_SIZE;
	} else {
		imageBuffer.count++;
	}

	// DEBUG
	if((imageBuffer.count < 0) || (imageBuffer.count > BUFFER_SIZE)) {
		printf("DEBUG: write_Image() count = %d\n", imageBuffer.count);fflush(stdout);
	}

	// DEBUG
	//printf("write: count = %d\n", imageBuffer.count);fflush(stdout);

	// unlock
	if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
		perror("write_Image(): failed to unlock mutex\n");
		exit(EXIT_FAILURE);
	}
}

int read_Image(IplImage* image) {
	// lock
	if(pthread_mutex_lock(&imageBuffer.lock) != 0) {
		perror("read_Image(): failed to lock mutex\n");
		exit(EXIT_FAILURE);
	}

	// init buffer if necessary
	if(!done) {
		init_ImageBuffer();
		done = TRUE;
	}

	// quit if buffer is empty
	if(imageBuffer.count == 0) {
		// unlock
		if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
			perror("read_Image(): failed to unlock mutex\n");
			exit(EXIT_FAILURE);
		}

		return EXIT_FAILURE;
	}

	// read from buffer
	*image = imageBuffer.data[imageBuffer.read_idx];

	// move read index to next position
	imageBuffer.read_idx = (imageBuffer.read_idx + 1) % BUFFER_SIZE;

	// decrease count
	imageBuffer.count--;

	// DEBUG
	if((imageBuffer.count < 0) || (imageBuffer.count > BUFFER_SIZE)) {
		printf("DEBUG: read_Image() count = %d\n", imageBuffer.count);fflush(stdout);
	}

	// DEBUG
	//printf("read: count = %d\n", imageBuffer.count);fflush(stdout);

	// unlock
	if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
		perror("read_Image(): failed to unlock mutex\n");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

