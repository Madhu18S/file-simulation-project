CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lcrypto

OBJS = main.o merkle.o

all: merkle_demo

merkle_demo: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.c merkle.h
	$(CC) $(CFLAGS) -c main.c

merkle.o: merkle.c merkle.h
	$(CC) $(CFLAGS) -c merkle.c

clean:
	rm -f $(OBJS) merkle_demo

.PHONY: all clean
