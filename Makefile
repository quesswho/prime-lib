CC     = gcc
CFLAGS = -Wall -Wextra -Iinclude

all: example/main.exe

example/main.exe: example/main.c src/prime.c include/prime.h
	$(CC) $(CFLAGS) example/main.c src/prime.c -o example/main.exe

clean:
	del /Q example\main.exe 2>nul || true
