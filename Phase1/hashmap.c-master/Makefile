CC = gcc
CFLAGS = -Wall
DEPS = hashmap.h
OBJ = hashmap.o test2.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

test2: $(OBJ)
	gcc $(CFLAGS) -o $@ $^
