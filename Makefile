FTPserver: FTPserver.c
	gcc FTPserver.c -o FTPserver utils.c

FTPclient: FTPclient.c
	gcc FTPclient.c -o FTPclient utils.c

clean:
	rm FTPclient FTPserver