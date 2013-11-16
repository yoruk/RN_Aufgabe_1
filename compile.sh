#!/bin/sh

gcc -Wall -g -c src/main.c -o src/main.o -pthread
gcc -Wall -g -c src/buffer.c -o src/buffer.o -pthread
gcc -Wall -g -c src/client.c -o src/client.o -pthread
gcc -Wall -g -c src/server_handler.c -o src/server_handler.o -pthread
gcc -Wall -g -c src/server.c -o src/server.o -pthread
gcc -o aufgabe_1 src/main.o src/buffer.o src/client.o src/server_handler.o src/server.o -lpthread -lm -lopencv_core -lopencv_highgui -lopencv_imgproc

