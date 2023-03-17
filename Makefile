#Project:   IPK Calculator Protocol
#Author:    Milan Takac - xtakac09
#2.BIT 2023

cc=GCC
CFLAGS= -std=c99 -pedantic -Wall -Wextra -Werror

ipkcp: ipkcp.c
	$(CC) $(CFLAGS) ipkcp.c -o ipkcp