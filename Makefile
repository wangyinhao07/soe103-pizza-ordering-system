# Build:  make            (Linux / MinGW)     produces ./pizza
# Test:   make test       run from this folder (the tests open tests/...)
# Clean:  make clean
# Windows one-click build: build.bat
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -pedantic -I.

SRC     = main.c utils.c fileio.c auth.c menu.c customer.c order.c stats.c
TARGET  = pizza

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRC) common.h utils.h fileio.h auth.h menu.h customer.h order.h stats.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

tests/test_utils: tests/test_utils.c utils.c utils.h common.h
	$(CC) $(CFLAGS) -o tests/test_utils tests/test_utils.c utils.c

tests/test_auth: tests/test_auth.c auth.c utils.c customer.c fileio.c \
		auth.h utils.h customer.h fileio.h common.h
	$(CC) $(CFLAGS) -o tests/test_auth tests/test_auth.c auth.c utils.c customer.c fileio.c

test: tests/test_utils tests/test_auth
	./tests/test_utils
	./tests/test_auth

clean:
	rm -f $(TARGET) $(TARGET).exe tests/test_utils tests/test_auth \
		tests/test_utils.exe tests/test_auth.exe tests/tmp_stdin.txt
