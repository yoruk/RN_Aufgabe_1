#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "buffer.h"
#include "global_const.h"


extern int run;

void* dummy_server(void* arg) {
	IplImage* image;

	sleep(1);

	printf("Dummy-Server: is running!\n");fflush(stdout);

	image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, PIXEL_SIZE);
	image->imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * PIXEL_SIZE;
//	cvNamedWindow("Dummy-Server", CV_WINDOW_AUTOSIZE);

	while(run) {
		read_Image(image);


//		cvShowImage("Dummy-Server", image);
//		if((cvWaitKey(5) & 255) == 27) {
//			break;
//		}
	}

	pthread_exit(NULL);
}
