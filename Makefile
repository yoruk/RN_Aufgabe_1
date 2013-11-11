CFLAGS = -Wall -g -pthread
LDFLAGS = -lpthread -lm -lopencv_core -lopencv_highgui -lopencv_imgproc
OBJ = src/main.o src/buffer.o src/client.o src/server_handler.o src/server.o
HEADER = src/global_const.h src/buffer.h src/client.h src/server_handler.h src/server.h
.PHONY: clean

aufgabe_1: $(OBJ)
	gcc $(LDFLAGS) -o $@ $(OBJ)

%.o: %.c $(HEADER)
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm aufgabe_1 $(OBJ)
	
all: aufgabe_1

