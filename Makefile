CC = clang
CFLAGS = -Wall -framework IOKit -framework CoreFoundation

all: macsleepmon

macsleepmon: main.c
	$(CC) $(CFLAGS) -o macsleepmon main.c

clean:
	rm -f macsleepmon