
- [CS144's documentation to the starter code](https://web.archive.org/web/20220827011711/https://cs144.github.io/doc/lab0)

# 2 Networking by hand

1. In a Web browser, visit http://cs144.keithw.org/hello and observe the result.

2. Now, you’ll do the same thing the browser does, by hand.
   
## 2.1 Fetch a Web page

```bash
telnet cs144.keithw.org http

# This tells the telnet program to open a reliable byte stream between your computer and another computer (named cs144.keithw.org), and with a particular service running on that computer: the “http” service, for the Hyper-Text Transfer Protocol, used by the World Wide Web

# Trying 104.196.238.229...
# Connected to cs144.keithw.org.
# Escape character is '^]'.

GET /hello HTTP/1.1

# This tells the server the path part of the URL. (The part starting with the third slash.)

Host: cs144.keithw.org
# This tells the server the host part of the URL. (The part between http:// and the third slash.)

Connection: close
# This tells the server that you are finished making requests, and it should close the connection as soon as it finishes replying

# Hit the Enter key one more time
# This sends an empty line and tells the server that you are done with your HTTP request.

# If all went well, you will see the same response that your browser saw, preceded
# by HTTP headers that tell the browser how to interpret the response.

# HTTP/1.1 200 OK
# Date: Sat, 26 Jul 2025 22:27:40 GMT
# Server: Apache
# Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
# ETag: "e-57ce93446cb64"
# Accept-Ranges: bytes
# Content-Length: 14
# Connection: close
# Content-Type: text/plain

# Hello, CS144!
# Connection closed by foreign host.

# 3. Assignment
# Use the above technique to fetch the URL http://cs144.keithw.org/lab0/sunetid, replacing sunetid with your own primary SUNet ID. 
GET /lab0/michaelid HTTP/1.1

# HTTP/1.1 200 OK
# Date: Sat, 26 Jul 2025 22:29:57 GMT
# Server: Apache
# X-You-Said-Your-SunetID-Was: michaelid
# X-Your-Code-Is: 54666
# Content-length: 113
# Vary: Accept-Encoding
# Connection: close
# Content-Type: text/plain

# Hello! You told us that your SUNet ID was "michaelid". Please see the HTTP headers (above) for your secret code.
# Connection closed by foreign host.
```


## 2.2 Send yourself an email

smtp: The “smtp” service refers to the Simple Mail Transfer Protocol, used to send email messages.

```bash
telnet 148.163.153.234 smtp
# Trying 148.163.153.234...
# Connected to 148.163.153.234.
# Escape character is '^]'.
# 220 mx0b-0026c301.pphosted.com ESMTP mfa-m0458491

# First step: identify your computer to the email server.
# Wait to see something like “250 ... Hello cardinal3.stanford.edu [171.67.24.75], pleased to meet you”.
HELO mycomputer.stanford.edu
# 250 mx0b-0026c301.pphosted.com Hello [183.158.115.117], pleased to meet you

# Next step: who is sending the email?
MAIL FROM: santaclaus@northpole.gov
# 250 2.1.0 Sender ok


# Next: who is the recipient? For starters, try sending an email message to yourself. 
# Replace sunetid with your own SUNet ID. If all goes well, you will see “250 2.1.5 Recipient ok.”
RCPT TO: michaelid@stanford.edu
# 550 5.7.1 Relaying denied


# It’s time to upload the email message itself. Type DATA to tell the server you’re
# ready to start. If all goes well, you will see “354 End data with <CR><LF>.<CR><LF>”.

# Now you are typing an email message to yourself. First, start by typing the headers
# that you will see in your email client. Leave a blank line at the end of the headers.
354 End data with <CR><LF>.<CR><LF>
From: sunetid@stanford.edu
To: sunetid@stanford.edu
Subject: Hello from CS144 Lab 0!


# Type the body of the email message—anything you like. When finished, end with a dot
# on a line by itself: . . Expect to see something like: “250 2.0.0 33h24dpdsr-1
# Message accepted for delivery”.

# Type QUIT to end the conversation with the email server. Check your inbox and
# spam folder to make sure you got the email.
```


## 2.3 Listening and connecting

You’ve seen what you can do with telnet: a client program that makes outgoing connections
to programs running on other computers. Now it’s time to experiment with being a simple
server: the kind of program that waits around for clients to connect to it.

1. In one terminal window, run netcat -v -l -p 9090 on your VM. You should see:
user@computer:~$ netcat -v -l -p 9090
Listening on [0.0.0.0] (family 0, port 9090)

```bash
netcat -v -l -p 9090
```

2. Leave netcat running. In another terminal window, run telnet localhost 9090
(also on your VM).
```bash
telnet localhost 9090
```

3. If all goes well, the netcat will have printed something like “Connection from localhost 53500 received!”.

4. Now try typing in either terminal window—the netcat (server) or the telnet (client).
Notice that anything you type in one window appears in the other, and vice versa.
You’ll have to hit for bytes to be transfered.

5. In the netcat window, quit the program by typing ctrl -C . Notice that the telnet
program immediately quits as well.



# 3 Writing a network program using an OS stream socket


## 3.1 Let's get started--fetching and building the starter code

1. The lab assignments will use a starter codebase called \Sponge." On your VM, run
git clone https://github.com/cs144/sponge to fetch the source code for the lab.

2. Optional: Feel free to backup your repository to a private GitHub/GitLab/Bitbucket repository 
(e.g., using the instructions at https://stackoverflow.com/questions/10065526/
github-how-to-make-a-fork-of-public-repository-private), but please make absolutely sure
that your work remains private.
3. Enter the Lab 0 directory: cd sponge
4. Create a directory to compile the lab software: mkdir build
5. Enter the build directory: cd build
6. Set up the build system: cmake ..
7. Compile the source code: make (you can run make -j4 to use four processors).
8. Outside the build directory, open and start editing the writeups/lab0.md file. 
9. This is the template for your lab writeup and will be included in your submission.


## 3.3  CS144's user-space TCP library

- [sponge](https://web.archive.org/web/20220827011711/https://cs144.github.io/doc/lab0/index.html)
- [CS144's documentation to the starter code](https://web.archive.org/web/20220827011711/https://cs144.github.io/doc/lab0)

![image](../images/CS144's%20user-space%20TCP%20library-class-hierarchy.png)

To support this style of programming, Sponge's classes wrap operating-system functions
(which can be called from C) in \modern" C++.
1. Using a Web browser, read over the documentation to the starter code at
https://cs144.github.io/doc/lab0.
2. Pay particular attention to the documentation for the FileDescriptor, Socket,
TCPSocket, and Address classes. (Note that a Socket is a type of FileDescriptor,
and a TCPSocket is a type of Socket.)
3. Now, find and read over the header files that describe the interface to these classes in
the libsponge/util directory: file descriptor.hh, socket.hh, and address.hh.







## 3.4 Writing webget

Let’s get started—setting up the repository on your VM and on GitHub
```bash
git clone https://github.com/cs144/minnow
# not exists, replace with 
- [PKUFlyingPig-2020](https://github.com/PKUFlyingPig/CS144-Computer-Network)
- [fei0319-2023](https://github.com/fei0319/CS144)
- [cs144-minnow-nju-2024](https://github.com/top-mind/cs144-minnow-nju)
```


### steps

- finish the webget.cc
- debug the webget.cc with vscode
- test with ./tests/webget_t.sh
  
```bash
cd cs144_sponge

cmake S . -B build
cmake --build build

cd build

#  run ./apps/webget excute under build directory
../tests/webget_t.sh 
# Function called: get_URL(cs144.keithw.org, /nph-hasher/xyzzy).
# Warning: get_URL() has not been implemented yet.
# HTTP/1.1 200 OK
# Content-type: text/plain

# 7SmXqWkrLKzVBCEalbSPqBcvs11Pw263K7x4Wv3JckI

make check_webget
# [100%] Testing webget...
# Test project /mnt/e/projects/computer_network/cs144_sponge/build
#     Start 28: t_webget
# 1/1 Test #28: t_webget .........................   Passed    1.10 sec

```

# 4 An in-memory reliable byte stream

### A reliable bidirectional byte stream

the ability to create a reliable bidirectional byte stream between two programs, one running on your computer, and the other on a different computer across the Internet (e.g., a Web server such as Apache or nginx, or the netcat program).

### Stream Socket

To your program and to the Web server, the socket looks like an ordinary file descriptor (similar to a file on disk, or to the stdin or stdout I/O streams). When two stream sockets are connected, any bytes written to one socket will eventually come out in the same order from the other socket on the other computer.


### Internet datagrams

Each datagram contains some metadata (headers) that specifies things like the source and destination addresses|what computer it came from, and what computer it's headed towards|as well as some payload data (up to about 1,500 bytes) to be delivered to the destination computer.

```bash
# build
make
# test
make check_lab0
```