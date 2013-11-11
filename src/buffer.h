#ifndef BUFFER_H_
#define BUFFER_H_

#include "global_const.h"

typedef struct {
	char data[IMAGE_SIZE];
} rawImage_t;

typedef struct {
	rawImage_t data[BUFFER_SIZE];
	unsigned int write_idx;
	unsigned int read_idx;
	unsigned int count;
	pthread_mutex_t lock;
} imageBuffer_t;

//void init_ImageBuffer(imageBuffer_t* imageBuffer);
void write_Image(imageBuffer_t* imageBuffer, rawImage_t* rawImage);
int read_Image(imageBuffer_t* imageBuffer, rawImage_t* rawImage);

#endif /* BUFFER_H_ */
