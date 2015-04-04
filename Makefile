all: evagg

evagg: evagg.c
	gcc -g -std=gnu99 -Wall -O2 -lutil evagg.c -o evagg
