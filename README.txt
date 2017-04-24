# simpleserver
CS485-004: Server/Client Project
Authors: Aaron Mueller, Connor VanMeter
=======
# LIST OF FILES
* README.txt: contains author names and a description of files
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
