#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "global_const.h"
#include "buffer.h"

//typedef struct {
//	char data[IMAGE_SIZE];
//} rawImage_t;


typedef struct {
	rawImage_t data[BUFFER_SIZE];
	unsigned int newest_image_idx;
	unsigned int oldest_image_idx;
	unsigned int count;
	pthread_mutex_t lock;
} imageBuffer_t;

static imageBuffer_t imageBuffer;
static int init_done = FALSE;

static void init_ImageBuffer() {
	imageBuffer.newest_image_idx = 0;
	imageBuffer.oldest_image_idx = 0;
	imageBuffer.count = 0;

	if(pthread_mutex_init(&imageBuffer.lock, NULL) != 0) {
		perror("Buffer: init_ImageBuffer(): ERROR, failed to init mutex");
		exit(EXIT_FAILURE);
	}
}

void write_Image(rawImage_t* image) {
	// lock
	if(pthread_mutex_lock(&imageBuffer.lock) != 0) {
		perror("Buffer: write_Image(): ERROR, failed to lock mutex");
		exit(EXIT_FAILURE);
	}

	// init buffer if never done before
	if(!init_done) {
		init_ImageBuffer();
		init_done = TRUE;
	}

	// drop oldest image if necessary,
	if(imageBuffer.newest_image_idx == imageBuffer.oldest_image_idx && imageBuffer.count >= BUFFER_SIZE) {
		imageBuffer.oldest_image_idx = (imageBuffer.oldest_image_idx + 1) % BUFFER_SIZE;
	}

	// write into buffer
	imageBuffer.data[imageBuffer.newest_image_idx] = *image;

	// move write index to next position
	imageBuffer.newest_image_idx = (imageBuffer.newest_image_idx + 1) % BUFFER_SIZE;

	if(imageBuffer.count != BUFFER_SIZE) {
		imageBuffer.count++;
	}

	// unlock
	if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
		perror("Buffer: write_Image(): ERROR, failed to unlock mutex");
		exit(EXIT_FAILURE);
	}
}

int read_Image(rawImage_t* image, bufferEntry_t* entry) {
	int tmp_oldest_image_idx;
	int tmp_offset;

	// lock
	if(pthread_mutex_lock(&imageBuffer.lock) != 0) {
		perror("Buffer: read_Image(): ERROR, failed to lock mutex");
		exit(EXIT_FAILURE);
	}

	// init buffer if never done before
	// and exit when done -> buffer is empty
	if(!init_done) {
		init_ImageBuffer();
		init_done = TRUE;

		if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
			perror("Buffer: read_Image(): ERROR, failed to unlock mutex");
			exit(EXIT_FAILURE);
		}

		return EXIT_FAILURE;
	}

	// have images been dropped since last access?
	if(imageBuffer.oldest_image_idx != entry->last_oldest_image_idx) {
		// yes, offset needs to be adjusted
		printf("yes\n");fflush(stdout);

		tmp_offset = ((imageBuffer.oldest_image_idx + entry->offset) % BUFFER_SIZE) - imageBuffer.oldest_image_idx;

	} else {
		// no
		printf("no\n");fflush(stdout);

		tmp_offset = entry->offset;
	}

	// current oldest_image_idx is always
	// the one we are working with
	tmp_oldest_image_idx = imageBuffer.oldest_image_idx;

	// DEBUG
	printf("debug:\n");fflush(stdout);
	printf("entry->last_oldest_image_idx = %d\n", entry->last_oldest_image_idx);fflush(stdout);
	printf("entry->offset = %d\n", entry->offset);fflush(stdout);
	printf("tmp_oldest_image_idx = %d\n", tmp_oldest_image_idx);fflush(stdout);
	printf("tmp_offset = %d\n", tmp_offset);fflush(stdout);
	printf("imageBuffer.newest_image_idx = %d\n", imageBuffer.newest_image_idx);fflush(stdout);

	// is the buffer empty
	// or the selected image not in the buffer?
	if(((tmp_oldest_image_idx + tmp_offset) % BUFFER_SIZE) == imageBuffer.newest_image_idx) {
		// yes, exit with error

		if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
			perror("Buffer: read_Image(): ERROR, failed to unlock mutex");
			exit(EXIT_FAILURE);
		}

		printf("exit\n");fflush(stdout);

		return EXIT_FAILURE;

	} else {
		// no, output image, adjust entry values
		*image = imageBuffer.data[(tmp_oldest_image_idx + tmp_offset) % BUFFER_SIZE];

		entry->last_oldest_image_idx = tmp_oldest_image_idx;
		entry->offset = (tmp_offset + 1) % BUFFER_SIZE;
	}

	// unlock
	if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
		perror("Buffer: write_Image(): ERROR, failed to unlock mutex");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

