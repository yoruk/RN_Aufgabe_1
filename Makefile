CFLAGS = -Wall -g -pthread
LDFLAGS = -lpthread -lm -lopencv_core -lopencv_highgui -lopencv_imgproc
OBJ = src/main.o
#HEADER = 
.PHONY: clean

aufgabe_1: $(OBJ)
	gcc $(LDFLAGS) -o $@ $(OBJ)

%.o: %.c $(HEADER)
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm aufgabe_1 $(OBJ)
	
all: aufgabe_1

