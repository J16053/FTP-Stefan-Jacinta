FTPserver: FTPserver.c
	gcc FTPserver.c -o FTPserver

FTPclient: FTPclient.c
	gcc FTPclient.c -o FTPclient

clean:
	rm FTPclient FTPserver