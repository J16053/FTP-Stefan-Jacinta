FTPserver: FTPserver.o utils.o
	mkdir -p bin
	gcc FTPserver.o utils.o -o bin/FTPserver

FTPclient: FTPclient.o utils.o
	mkdir -p bin
	gcc FTPclient.o utils.o -o bin/FTPclient
	
FTPserver.o: FTPserver.c utils.h
	gcc -c FTPserver.c
		
FTPclient.o: FTPclient.c utils.h
	gcc -c FTPclient.c
	
utils.o: utils.c utils.h
	gcc -c utils.c

clean:
	rm FTPclient FTPserver *.o
	