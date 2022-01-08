CC=gcc
CFLAGS=-g -Wall -D_FILE_OFFSET_BITS=64 -lm -s -lpthread
LDFLAGS= -lfuse -lm -s -lpthread

OBJ=tfs.o block.o

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

tfs: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o tfs

.PHONY: clean
clean:
	rm -f *.o tfs
