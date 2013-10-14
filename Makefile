CFLAGS = -Wall -g -pthread
LDFLAGS = -lpthread
OBJ = src/main.o
#HEADER = 
.PHONY: clean

world: $(OBJ)
	gcc $(LDFLAGS) -o $@ $(OBJ)

%.o: %.c $(HEADER)
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm world $(OBJ)
	
all: world

