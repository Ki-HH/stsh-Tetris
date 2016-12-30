.PHONY: all

all: tetris

tetris: tetris.c
	gcc -D__STANDALONE__-Wall -g -o tetris tetris.c
