CC=gcc
CFLAGS=-g -ggdb -std=gnu99 -O

quickjson: quickjson.c
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -rf quickjson

.PHONY: clean

