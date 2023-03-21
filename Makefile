# Project 1 - IPK calculator protocol
# Author: Milan Takac - xtakac09
# 2BIT VUT FIT (BUT)

cc=GCC
CFLAGS= -std=c99 -pedantic -Wall -Wextra -Werror

ipkcpc: ipkcpc.c
	$(CC) $(CFLAGS) ipkcpc.c -o ipkcpc