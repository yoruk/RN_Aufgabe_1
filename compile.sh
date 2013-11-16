#!/bin/sh

gcc -Wall -g -pthread -c src/main.c -o src/main.o
gcc -Wall -g -pthread -c src/buffer.c -o src/buffer.o
gcc -Wall -g -pthread -c src/client.c -o src/client.o
gcc -Wall -g -pthread -c src/server_handler.c -o src/server_handler.o
gcc -Wall -g -pthread -c src/server.c -o src/server.o
gcc -lpthread -lm -lopencv_core -lopencv_highgui -lopencv_imgproc -o aufgabe_1 src/main.o src/buffer.o src/client.o src/server_handler.o src/server.o

