# Build: make        (Linux / MinGW)       Windows: build.bat
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -pedantic
SRC     = main.c utils.c fileio.c auth.c menu.c customer.c order.c stats.c
TARGET  = pizza

$(TARGET): $(SRC) *.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) $(TARGET).exe
