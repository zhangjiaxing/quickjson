CC=gcc
CFLAGS=-g -ggdb -std=gnu99 -Wall -Wformat=0

quickjson: quickjson.c
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -rf quickjson *.o

.PHONY: clean

