#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "global_const.h"
#include "buffer.h"


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

	// DEBUG
//	printf("----- debug: write_image() -----\n");fflush(stdout);
//	printf("imageBuffer.newest_image_idx = %d\n", imageBuffer.newest_image_idx);fflush(stdout);
//	printf("imageBuffer.oldest_image_idx = %d\n", imageBuffer.oldest_image_idx);fflush(stdout);
//	printf("imageBuffer.count = %d\n", imageBuffer.count);fflush(stdout);

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


	// DEBUG
//	printf("----- debug: read_image() -----\n");fflush(stdout);
//	printf("entry->last_oldest_image_idx = %d\n", entry->last_oldest_image_idx);fflush(stdout);
//	printf("entry->offset = %d\n", entry->offset);fflush(stdout);
//	printf("imageBuffer.newest_image_idx = %d\n", imageBuffer.newest_image_idx);fflush(stdout);
//	printf("imageBuffer.oldest_image_idx = %d\n", imageBuffer.oldest_image_idx);fflush(stdout);
//	printf("imageBuffer.count = %d\n", imageBuffer.count);fflush(stdout);

	// have images been dropped since last access?
	if(imageBuffer.oldest_image_idx != entry->last_oldest_image_idx) {
		// yes, offset needs to be adjusted

		// DEBUG
//		printf("oldest image has moved!\n");fflush(stdout);

		tmp_offset = ((imageBuffer.oldest_image_idx + entry->offset) % BUFFER_SIZE) - imageBuffer.oldest_image_idx;

	} else {
		// no

		// DEBUG
//		printf("oldest image is still the same\n");fflush(stdout);

		tmp_offset = entry->offset;
	}

	// current oldest_image_idx is always
	// the one we are working with
	tmp_oldest_image_idx = imageBuffer.oldest_image_idx;

	// DEBUG
//	printf("tmp_oldest_image_idx = %d\n", tmp_oldest_image_idx);fflush(stdout);
//	printf("tmp_offset = %d\n", tmp_offset);fflush(stdout);

	// explanation for the if() below:
	//
	// 1. if tmp_oldest_image_idx == imageBuffer.newest_image_idx means,
	//    that a wrap around in the buffer did happen,
	//    in this case the tmp_offset isn't allowed to be larger than the buffer size
	//
	// 2. if tmp_oldest_image_idx != imageBuffer.newest_image_idx means,
	//    that no wrap around did happen
	//    in this case the tmp_oldest_image_idx + tmp_offset
	//    isn't allowed to reach larger indexes than imageBuffer.newest_image_idx

	// is the selected image in the buffer?
	if( ((tmp_oldest_image_idx == imageBuffer.newest_image_idx) && tmp_offset >= BUFFER_SIZE)
			|| ((tmp_oldest_image_idx != imageBuffer.newest_image_idx) &&  ((tmp_oldest_image_idx + tmp_offset) % BUFFER_SIZE) >= imageBuffer.newest_image_idx)) {

			// no, exit with error

			if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
				perror("Buffer: read_Image(): ERROR, failed to unlock mutex");
				exit(EXIT_FAILURE);
			}

			// DEBUG
			printf("the wanted image is not in the buffer!, exit\n");fflush(stdout);

			return EXIT_FAILURE;

	} else {
		// yes, output image, adjust entry values
		*image = imageBuffer.data[(tmp_oldest_image_idx + tmp_offset) % BUFFER_SIZE];

		entry->last_oldest_image_idx = tmp_oldest_image_idx;
		entry->offset = (tmp_offset + 1) % BUFFER_SIZE;
	}

	// DEBUG
//	printf("entry->last_oldest_image_idx = %d\n", entry->last_oldest_image_idx);fflush(stdout);
//	printf("entry->offset = %d\n", entry->offset);fflush(stdout);

	// unlock
	if(pthread_mutex_unlock(&imageBuffer.lock) != 0) {
		perror("Buffer: write_Image(): ERROR, failed to unlock mutex");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

