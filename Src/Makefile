CC = gcc
CFLAGS = -Wall -g -o
GTK = `pkg-config --cflags --libs gtk+-2.0 gthread-2.0`

all: view

view: raspberryView.c image_recv_T.c raspberryView.h
	$(CC) $(CFLAGS) view raspberryView.c image_recv_T.c $(GTK)

clean:
	rm view
