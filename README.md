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

A sserver library has been constructed that can be used by any program, not
just the four programs included in the zip file. The library routines are as
follows:


Each of these returns 0 on success and -1 on failure.

NOTE: MachineName, TCPport, and SecretyKey are NOT included in these routines.
These three pieces are already handled in the main and would be redundant
in the routines.

# CLIENT APPLICATIONS
* ssSet _MachineName TCPport SecretKey variableName value_
ssSet takes an environment-variable name and a value. THe server set the
environment variable to that value, which must not exceed 100 bytes. The
client prints nothing unless the command fails, in which case it prints failed.

* ssGet _MachineName TCPport SecretKey variableName_
ssGet takes an environment-variable name as its detail. The server reports the
contents of that environment variable, or failure if such as variable is not set.
The client prints the contents it receives from the server, or failed.

* ssDigest _MachineName TCPport SecretKey value_
ssDigest returns the results of the invocation:
sh -c 'echo `/bin/hostname` value | /usr/bin/md5sum'
The result is guaranteed to be 100 bytes or less. The client prints the results
it receives from the server, or failed.

* ssRun _MachineName TCPport SecretKey program_
ssRun takes a program name as its detail. The server runs the program and
sends the first 100 bytes of its output back to the client. Only certain
programs are valid requets; the others must be rejected. THe valid programs are:
	inet   /sbin/ifconfig -a
	hosts  /bin/cat /etc/hosts
	uptime /usr/bin/uptime
The client prints the result it receives from the server, or failed.

# MESSAGES FROM CLIENT TO SERVER
All messages from the client to the server have this format:
* Bytes 0-3: A 4-byte unsigned int containing _SecretKey_ in network byte order
* Byte 4: A 1-byte unsiged int containing the type of request set(0), get (1),
  digest(2), run(3).
* Bytes 5-7: Three bytes of padding ("cat")

The remaining bytes are command-specific:
* Set request
	* Bytes 8-23: a null-terminated variable name, no longer than 15 chars
	* Bytes 24-27: A 4-byte unsig int (in net order) giving length of value,
	  which must not exceed 100, including the term null (for string)
	* Bytes 28...: The value itself with length bytes sent
* Get request
	* Bytes 8-23: a null-term variable name, no longer than 15 chars
* Digest request
	* Bytes 8-11: a 4-byte unsig int (int net order) giving length of value,
	  which must not exceed 100
	* Bytes 12...: The value itself with length bytes sent
* Run request
	* Byte 8-15: an 8-byte string (null term) holding a valid prog name

# MESSAGE FROM SERVER TO CLIENT
If a client sends a request with an invalid secret key, the server may close
the connection without returning any message at all.

All messages from the server to the client have this format:
* Byte 0: A 1-byte return code: 0 for success, -1 for failure
* Bytes 1-3: Three bytes of padding, with arbitrary values

The remaining bytes are command-specific:
* Set respond
	* No further data
* Get, Digest, Run response
	* Bytes 4-7: A 4-byte unsig int (in net order) giving length of value,
	  which must not exceed 100, including the term null (for string)
	* Bytes 8...: The value itself with length bytes sent

# LIMITATIONS/BUGS
The following assumptions were made:
* Machine Names (DNS or IP addresses) have a maximum length of 40 characters
* Data can be either binary or text
* Server does not provide service unless the client and server SecretyKey match
* The SecretKey is an unsigned int in the range of 0 and 2^32 - 1
