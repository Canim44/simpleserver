# simpleserver
CS485-004: Server/Client Project
Authors: Aaron Mueller, Connor VanMeter
=======

# DESCRIPTION
`simpleserver` is a project for CS485G. It consists of a server and four
client programs (descriptions below). This project was coded in C.

# LIST OF FILES
* README.txt: contains a description of files, project, and limitations
* Makefile: file containg directives used with `make`
* csapp.c: Provided socket syscalls and RIO library calls
* csapp.h: header for csapp.h
* simpled.c: server code containing library
* ssSet.c: set an environmental variable to a value
* ssGet.c: get an environmental variable's value
* ssDigest.c: returns the result of
sh -c 'echo `/bin/hostname` value | /usr/bin/md5sum'
* ssRun.c: run a valid program (inet, hosts, uptime)

NOTE: csapp.c and csapp.h are modified from code at
http://csapp.cs.cmu.edu/public/code.html

# HOW TO RUN/EXIT THIS PROJECT
1. Use `make` to compile server and client programs
2. Run `./simpled <port> <secretKey>` to start the server
3. Run client program (ssSet, ssGet, ssDigest, ssRun) to communicate with server
4. Type `ctrl-C` to exit the server

# SERVER
The server will print out the following information for every valid request:
* Secret key = _SecretKey_
where _SecretKey_ is the secret key contained in the request
* Request type = _type_
where _type_ is one of set, get, digest, run
* Detail = _detail_
where _detail_ is specific to the particular request type
* Completion = _status_
where _status_ is either success or failure

If the secret key is wrong, only the SecretKey line is printed. The server
immediately clsoes the connection to the client.

# LIMITATIONS/BUGS
The following assumptions were made:
* Machine Names (DNS or IP addresses) have a maximum length of 40 characters
* Data can be either binary or text
* Server does not provide service unless the client and server SecretyKey match
* The SecretKey is an unsigned int in the range of 0 and 2^32 - 1
