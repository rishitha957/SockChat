CC = gcc
FLAGS = -pthread

all: server client 

server: server.c
	$(CC) $(FLAGS) server.c -o server 

client: client.c
	$(CC) $(FLAGS) client.c -o client

clean:
	rm -rf server client 
