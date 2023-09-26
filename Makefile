CC=gcc

quickjson: quickjson.c
	$(CC) -g -ggdb -o $@ $^
