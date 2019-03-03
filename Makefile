FTPserver: FTPserver.o utils.o
	gcc FTPserver.o utils.o -o FTPserver

FTPclient: FTPclient.o utils.o
	gcc FTPclient.o utils.o -o FTPclient
	
FTPserver.o: FTPserver.c utils.h
	gcc -c FTPserver.c
		
FTPclient.o: FTPclient.c utils.h
	gcc -c FTPclient.c
	
utils.o: utils.c utils.h
	gcc -c utils.c

clean:
	rm FTPclient FTPserver *.o
	