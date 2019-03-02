# FTP-Stefan-Jacinta
Networks Project 1: File Transfer Protocol (FTP)

This project implements a simplified version of FTP using the client/server model and a persistent TCP connection used as a control channel. We used [Thomas' template code](https://github.com/tpoetsch/concurrent_client_server) to set up a server that can handle simultaneous users' requests. Once the client decides to start a data stream, which could be either uploading a file to the server or downloading a file from the server, a new TCP connection is established on a different port for the data transfer. Once the file transfer ends, the TCP connection of the data transfer is closed.

Usage
---
To build the FTP server:
```
$ make FTPserver
```
Once the server has been compiled, it can be run using the command:
```
$ ./FTPserver
```

To build the FTP client:
```
$ make FTPclient
```
Once the client has been compiled, it can be run using the command:
```
$ ./FTPclient [ServerIP] [ServerPort]
```
Where [ServerIP] is 127.0.0.1 if the server is being run on the same host, and [ServerPort] is 9999.

Once both the server and the client are up and running and the connection has been set up, the client will see a prompt that looks like this:
```
ftp >
```
Commands to implement
---
- [x] **USER username** with this command the client gets to identify which user is trying to login to the FTP server. This is mainly used for authentication, since there might be different access controls specified for each user.

- [x] **PASS password** once the client issues a USER command to the server and gets the ”331 Username OK, password required” status code from the server. The client needs to authen- ticate with the user password by issuing a PASS command followed by the user password.

- [ ] **PUT filename** this command is used to upload a file named filename from the current client directory to the current server directory. This command should trigger the transfer of the file all the way to the server. Remember that a separate TCP connection to the server needs to be opened for the data transfer.

- [ ] **GET filename** this command is used to download a file named filename from the current server directory to the current client directory. Similar to PUT, a new separate TCP connection to the server needs to be opened for the data transfer.

- [ ] **LS** this command is used to list all the files under the current server directory
- [x] **!LS** this command is used to list all the files under the current client directory
- [ ] **CD** this command is used to change the current server directory
- [x] **!CD** this command is used to change the current client directory
- [ ] **PWD** this command displays the current server directory
- [x] **!PWD** this command displays the current client directory
- [x] **QUIT** this command quits the FTP session and closes the control TCP connection
