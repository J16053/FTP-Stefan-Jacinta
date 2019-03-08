FTPserver: FTPserver.o utils.o
	mkdir -p bin
	gcc -std=gnu99 FTPserver.o utils.o -o bin/FTPserver

FTPclient: FTPclient.o utils.o
	mkdir -p bin
	gcc -std=gnu99 FTPclient.o utils.o -o bin/FTPclient
	
FTPserver.o: FTPserver.c utils.h
	gcc -std=gnu99 -c FTPserver.c
		
FTPclient.o: FTPclient.c utils.h
	gcc -std=gnu99 -c FTPclient.c
	
utils.o: utils.c utils.h
	gcc -std=gnu99 -c utils.c

clean:
	rm *.o
	
