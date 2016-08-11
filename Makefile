CC = gcc

all: file_server file_client 

file_server: file_server.o
	$(CC) file_server.c -o $@

file_client: file_client.o   
	$(CC) file_client.c -o $@ 

clean: 
	rm -f *.o file_server file_client
tar: 
	tar -cvf allFiles.tar file_*.c Makefile
