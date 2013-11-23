#ifndef BUFFER_H_
#define BUFFER_H_

#include "global_const.h"

typedef struct {
	char data[IMAGE_SIZE];
} rawImage_t;

typedef struct {
	int last_oldest_image_idx;
	int last_read_idx;
	int offset;
} bufferEntry_t;

void write_Image(rawImage_t* rawImage);
int read_Image(rawImage_t* image, bufferEntry_t* entry);

#endif /* BUFFER_H_ */
