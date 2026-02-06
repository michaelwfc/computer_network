


# 2 Networking by hand
Let's get started with using the network. You are going to do two tasks by hand: 
- retrieving a Web page (just like a Web browser) 
- sending an email message (like an email client). 
Both of these tasks rely on a networking abstraction called **a reliable bidirectional byte stream**: 
you'll type a sequence of bytes into the terminal, and the same sequence of bytes will eventually
be delivered, in the same order, to a program running on another computer (a server). The
server responds with its own sequence of bytes, delivered back to your terminal.

   
## 2.1 Fetch a Web page

1. In a Web browser, visit http://cs144.keithw.org/hello and observe the result.

2. Now, you’ll do the same thing the browser does, by hand.

```bash
# (a) On your VM, run
telnet cs144.keithw.org http

# This tells the telnet program to open a reliable byte stream between your computer and another computer (named cs144.keithw.org), and with a particular service running on that computer: the “http” service, for the Hyper-Text Transfer Protocol, used by the World Wide Web

# Trying 104.196.238.229...
# Connected to cs144.keithw.org.
# Escape character is '^]'.

# The computer's name has a numerical equivalent (104.196.238.229, an Internet Protocol v4 address),
# and so does the service's name (80, a TCP port number).
# If you need to quit, hold down ctrl and press ] , and then type close and enter


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

# the 1st on 2025
HTTP/1.1 200 OK
Date: Sat, 26 Jul 2025 22:27:40 GMT
Server: Apache
Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
ETag: "e-57ce93446cb64"
Accept-Ranges: bytes
Content-Length: 14
Connection: close
Content-Type: text/plain

Hello, CS144!
Connection closed by foreign host.


# the 2nd on 2026
HTTP/1.1 200 OK
Date: Sat, 31 Jan 2026 22:04:00 GMT
Server: Apache
Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
ETag: "e-57ce93446cb64"
Accept-Ranges: bytes
Content-Length: 14
Content-Type: text/plain

Hello, CS144!
Connection closed by foreign host.

# 3. Assignment
# Use the above technique to fetch the URL http://cs144.keithw.org/lab0/sunetid, replacing sunetid with your own primary SUNet ID. 
telnet cs144.keithw.org http

GET /lab0/michaelid HTTP/1.1
Host: cs144.keithw.org
Connection: close

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


cs144@cs144vm:~/computer_network/docs/c++notes$ telnet cs144.keithw.org http
Trying 104.196.238.229...
Connected to cs144.keithw.org.
Escape character is '^]'.
GET /lab0/michaelid HTTP/1.1
Host: cs144.keithw.org
Connection: close

HTTP/1.1 200 OK
Date: Sat, 31 Jan 2026 22:22:30 GMT
Server: Apache
X-You-Said-Your-SunetID-Was: michaelid
X-Your-Code-Is: 464428
Content-length: 113
Vary: Accept-Encoding
Connection: close
Content-Type: text/plain

Hello! You told us that your SUNet ID was "michaelid". Please see the HTTP headers (above) for your secret code.
Connection closed by foreign host

```


## 2.2 Send yourself an email

Now that you know how to fetch a Web page, it's time to send an email message, again using
a reliable byte stream to a service running on another computer.




```bash
# 1. Log in to cardinal.stanford.edu (to make sure you are on Stanford's network), then run
telnet 148.163.153.234 smtp
# smtp: The “smtp” service refers to the Simple Mail Transfer Protocol, used to send email messages.

# Trying 148.163.153.234...
# Connected to 148.163.153.234.
# Escape character is '^]'.
# 220 mx0b-0026c301.pphosted.com ESMTP mfa-m0458491



# check0(2025 Fall)
# 1. SSH to sunetid@cardinal.stanford.edu (to make sure you are on Stanford’s network), then run 
telnet 67.231.149.169 smtp




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

You’ve seen what you can do with `telnet`: a client program that makes outgoing connections to programs running on other computers. Now it’s time to experiment with being a simple server: the kind of program that waits around for clients to connect to it.

1. In one terminal window, run `netcat -v -l -p 9090` on your VM. 

```bash
netcat -v -l -p 9090

# You should see:
# user@computer:~$ netcat -v -l -p 9090
# Listening on [0.0.0.0] (family 0, port 9090)


# Different Netcat Versions
There are different implementations of netcat:
# 1. Traditional netcat (netcat-traditional)
# 2. OpenBSD netcat (netcat-openbsd) 

# Why the difference?
# OpenBSD version: Modern, simplified syntax - port number comes after -l
# Traditional version: Older syntax - requires explicit -p flag for port

# Check which version you have:
cs144@cs144vm:~$ nc -h 2>&1 |head -5
# If it mentions OpenBSD -> use: nc -l 9090
# If it mentions GNU/traditional -> use: nc -l -p 9090

OpenBSD netcat (Debian patchlevel 1.228-1)
usage: nc [-46CDdFhklNnrStUuvZz] [-I length] [-i interval] [-M ttl]
          [-m minttl] [-O length] [-P proxy_username] [-p source_port]
          [-q seconds] [-s sourceaddr] [-T keyword] [-V rtable] [-W recvlimit]
          [-w timeout] [-X proxy_protocol] [-x proxy_address[:port]]

```

2. Leave `netcat` running. In another terminal window, run `telnet` localhost 9090
(also on your VM).
```bash
telnet localhost 9090
```

3. If all goes well, the `netcat` will have printed something like “Connection from localhost 53500 received!”.

4. Now try typing in either terminal window—the `netcat` (server) or the `telnet` (client).
Notice that anything you type in one window appears in the other, and vice versa.
You’ll have to hit for bytes to be transfered.

5. In the `netcat` window, quit the program by typing `ctrl -C` . Notice that the `telnet` program immediately quits as well.

### telnet

Telnet is an older protocol for remote terminal access.

Basic Function:
- Remote login protocol from the 1960s
- Connects to a remote machine and provides a terminal session
- Everything sent in plain text (not encrypted - not secure!)

Modern Usage:
Today, telnet is mainly used for:
- Testing network connectivity to specific ports
- Debugging network services
- Simple protocol testing (HTTP, SMTP, etc.)

NOT used for actual remote login anymore (SSH replaced it for security)


Examples:
```bash
# 1. Test Web Server:
telnet google.com 80
# Once connected, type:
GET / HTTP/1.1
Host: google.com

# You'll see the HTTP response


# 2. Test if Port is Open:
telnet localhost 9090
# If it connects -> port is open
# If it fails -> port is closed or nothing listening
```


### netcat

Netcat (nc) is called the "Swiss Army knife" of networking. It's a simple utility that can:
Basic Function:
- Read from and write to network connections using TCP or UDP
- Acts as either a client (connects to) or server (listens on) a port

Common Uses:
```bash
# 1. Simple Server (Listening Mode):
nc -l 9090
# Listens on port 9090, waits for incoming connections


# 2. Simple Client (Connect Mode):
nc localhost 9090
# Connects to port 9090 on localhost

# 3. File Transfer:
# Receiver (Terminal 1):
nc -l 9090 > received_file.txt

# Sender (Terminal 2):
nc localhost 9090 < file_to_send.txt

# 4. Port Scanning:
nc -zv google.com 80
# Check if port 80 is open

# 5. Simple Chat:
# Person A:
nc -l 9090

# Person B:
nc <Person_A_IP> 9090
# Now they can type messages back and forth

```

# 3 Writing a network program using an OS stream socket

In the next part of this warmup lab, you will write a short program that fetches a Web page over the Internet. You will make use of a feature provided by the Linux kernel, and by most other operating systems: the ability to create **a reliable bidirectional byte stream** between two programs, one running on your computer, and the other on a different computer across the Internet (e.g., a Web server such as `Apache` or `nginx`, or the `netcat` program).


This feature is known as a `stream socket`. To your program and to the Web server, the socket looks like an ordinary file descriptor (similar to a file on disk, or to the stdin or stdout I/O streams). When two stream sockets are connected, any bytes written to one socket will eventually come out in the same order from the other socket on the other computer.

In reality, however, the Internet doesn't provide a service of reliable byte-streams. Instead, the only thing the Internet really does is to give its "best effort" to deliver short pieces of data, called **Internet datagrams**, to their destination. Each datagram contains some metadata (`headers`) that specifies things like the source and destination addresses---what computer it came from, and what computer it's headed towards---as well as some `payload` data (up to about 1,500 bytes) to be delivered to the destination computer.

Although the network tries to deliver every datagram, in practice datagrams can be
(1) lost,
(2) delivered out of order,
(3) delivered with the contents altered, or even 
(4) duplicated and delivered more than once. 
It's normally the job of the operating systems on either end of the connection to turn "**best-effort datagrams**" (the abstraction the Internet provides) into "**reliable byte streams**" (the abstraction that applications usually want).

The two computers have to cooperate to make sure that each byte in the stream eventually gets delivered, in its proper place in line, to the stream socket on the other side. They also have to tell each other how much data they are prepared to accept from the other computer, and make sure not to send more than the other side is willing to accept. All this is done using an agreed-upon scheme that was set down in 1981, called **the Transmission Control Protocol, or TCP**.

In this lab, you will simply use the operating system's pre-existing support for the Transmission Control Protocol. 
You'll write a program called "**webget**" that creates a TCP stream socket, connects to a Web server, and fetches a page|much as you did earlier in this lab. In future labs, you'll implement the other side of this abstraction, by implementing the Transmission
Control Protocol yourself to create a reliable byte-stream out of not-so-reliable datagrams.

## 3.1 Let's get started--fetching and building the starter code

- [sponge](https://web.archive.org/web/20220827011711/https://cs144.github.io/doc/lab0/index.html)
- [CS144's documentation to the starter code](https://web.archive.org/web/20220827011711/https://cs144.github.io/doc/lab0)

1. The lab assignments will use a starter codebase called "**Sponge**" On your VM, run
```bash
git clone https://github.com/cs144/sponge # not exists, replace with 
# git clone https://github.com/cs144/minnow

- [PKUFlyingPig-2020](https://github.com/PKUFlyingPig/CS144-Computer-Network)
- [fei0319-2023](https://github.com/fei0319/CS144)
- [cs144-minnow-nju-2024](https://github.com/top-mind/cs144-minnow-nju)
```

to fetch the source code for the lab.

2. Optional: Feel free to backup your repository to a private GitHub/GitLab/Bitbucket repository 
(e.g., using the instructions at https://stackoverflow.com/questions/10065526/
github-how-to-make-a-fork-of-public-repository-private), but please make absolutely sure
that your work remains private.
3. Enter the Lab 0 directory: cd sponge
4. Create a directory to compile the lab software:` mkdir build`
5. Enter the build directory: `cd build`
6. Set up the build system: `cmake ..`
7. Compile the source code: `make` (you can run `make -j4` to use four processors).
8. Outside the build directory, open and start editing the writeups/lab0.md file. 
9. This is the template for your lab writeup and will be included in your submission.

## 3.2 Modern C++: mostly safe but still fast and low-level

The lab assignments will be done in a contemporary C++ style that uses recent (2011)
features to program as safely as possible. This might be different from how you have been
asked to write C++ in the past. For references to this style, please see the C++ Core
Guidelines (http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).


The basic idea is to make sure that every object is designed to have the smallest possible public interface, has a lot of internal safety checks and is hard to use improperly, and knows how to clean up after itself. 
We want to avoid "paired" operations (e.g. `malloc/free`, or `new/delete`), where it might be possible for the second half of the pair not to happen (e.g., if a function returns early or throws an exception). 
Instead, operations happen in the constructor to an object, and the opposite operation happens in the destructor. This style is called **"Resource acquisition is initialization," or RAII.**


In particular, we would like you to:
• Use the language documentation at https://en.cppreference.com as a resource.
• Never use `malloc()` or `free()`.
• Never use `new` or `delete`.
• Essentially never use `raw pointers (*)`, and use `"smart" pointers` (unique ptr or shared ptr) only when necessary. (You will not need to use these in CS144.)
• Avoid templates, threads, locks, and virtual functions. (You will not need to use these
in CS144.)
• Avoid `C-style strings` (char *str) or string functions (strlen(), strcpy()). These are pretty error-prone. Use a std::string instead.
• Never use `C-style casts` (e.g., (FILE *)x). Use a C++ static cast if you have to (you generally will not need this in CS144).
• Prefer passing function arguments by `const reference` (e.g.: const Address & address).
• Make every variable `const` unless it needs to be mutated.
• Make every method `const` unless it needs to mutate the object.
• Avoid `global variables`, and give every variable the smallest scope possible.
• Before handing in an assignment, please run `make format` to normalize the coding style.

• Before handing in an assignment, 
run `cmake--build build--target tidy` suggestions on how to improve the code related to C++ programming practices,
and for `cmake--build build--target format` to format the code consistently.

### On using Git: 
The labs are distributed as Git (version control) repositories a way of documenting changes, checkpointing versions to help with debugging, and tracking the provenance of source code. Please make frequent small commits as you work, and use commit messages that identify what changed and why. 
The Platonic ideal is that each commit should compile and should move steadily towards more and more tests passing. Making small "semantic" commits helps with debugging (it's much easier to debug if each commit compiles and the message describes one clear thing that the commit does) and protects you against claims of cheating by documenting your steady progress over time and it's a useful skill that will help in any career that includes software development. 

The graders will be reading your commit messages to understand how you developed your solutions to the labs. If you haven't learned how to use Git, please do ask for help at the CS144 office hours or consult a tutorial (e.g., https://guides.github.com/introduction/git-handbook). 
Finally,you are welcome to store your code in a private repository on GitHub, GitLab, Bitbucket, etc., but please make sure your code is not publicly accessible.

## 3.3  Reading the Sponge documentation
CS144's user-space TCP library

![image](../../images/CS144's%20user-space%20TCP%20library-class-hierarchy.png)

To support this style of programming, Sponge's classes wrap operating-system functions(which can be called from C) in modern" C++.
1. Using a Web browser, read over the documentation to the starter code at https://cs144.github.io/doc/lab0.
2. Pay particular attention to the documentation for the `FileDescriptor`, `Socket`, `TCPSocket`, and `Address` classes. (Note that a Socket is a type of FileDescriptor, and a TCPSocket is a type of Socket.)
3. Now, find and read over the header files that describe the interface to these classes in the `libsponge/util` directory: `file_descriptor.hh`, `socket.hh`, and `address.hh`.


## 3.4 Writing webget
It's time to implement `webget`, a program to fetch Web pages over the Internet using the operating system's TCP support and stream-socket abstraction|just like you did by hand earlier in this lab.

1. From the build directory, open the file `../apps/webget.cc` in a text editor or IDE.
2. In the `get_URL` function,find the comment starting "// Your code here."
3. Implement the simple Web client as described in this file, using the format of an HTTP (Web) request that you used earlier. Use the `TCPSocket` and `Address` classes.
4. Hints:
• Please note that in HTTP, each line must be ended with "`\r\n`" (it's not sufficient to use just "`\n`" or `endl`).
• Don't forget to include the "`Connection: close`" line in your client's request. This tells the server that it shouldn't wait around for your client to send any more requests after this one. Instead, the server will send one reply and then will immediately end its outgoing bytestream (the one from the server's socket to your socket). You'll discover that your incoming byte stream has ended because your socket will reach "`EOF`" (end of file) when you have read the entire byte stream coming from the server. That's how your client will know that the server has finished its reply.
• Make sure to read and print all the output from the server until the socket reaches "`EOF`" (end of file)---a single call to read is not enough.
• We expect you'll need to write about ten lines of code.

5. Compile your program by running `make .` If you see an error message, you will need to fixx it before continuing.

6. Test your program by running `./apps/webget cs144.keithw.org /hello` . 
   How does this compare to what you see when visiting http://cs144.keithw.org/hello in a Web browser? 
   How does it compare to the results from Section 2.1? Feel free to experiment---test it with any http URL you like!


7. When it seems to be working properly, run `make check webget` to run the automated test. Before implementing the get URL function, you should expect to see the
following:
```bash
1/1 Test #25: lab0_webget ......................***Failed 0.00 sec
Function called: get_URL(cs144.keithw.org, /hasher/xyzzy).
Warning: get_URL() has not been implemented yet.
ERROR: webget returned output that did not match the tests expectations
```

After completing the assignment, you will see:
```bash
4/4 Test #4: lab0_webget ...................... Passed 0.14 sec
100% tests passed, 0 tests failed out of 4
```

8. The graders will run your webget program with a different hostname and path than `make check` runs---so make sure it doesn't only work with the hostname and path used by `make check`.

## Solution

- finish the webget.cc
- debug the webget.cc with vscode
- test with ./tests/webget_t.sh

### Debug & Test
```bash
cd sponge

make -p build
cd build

cs144@cs144vm:~/computer_network/sponge/build$ cmake ..
# CMake Deprecation Warning at CMakeLists.txt:1 (cmake_minimum_required):
#   Compatibility with CMake < 3.10 will be removed from a future version of
#   CMake.

#   Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
#   to tell CMake that the project requires at least <min> but has been updated
#   to work with policies introduced by <max> or earlier.


# -- The C compiler identification is GNU 14.2.0
# -- The CXX compiler identification is GNU 14.2.0
# -- Detecting C compiler ABI info
# -- Detecting C compiler ABI info - done
# -- Check for working C compiler: /usr/bin/cc - skipped
# -- Detecting C compile features
# -- Detecting C compile features - done
# -- Detecting CXX compiler ABI info
# -- Detecting CXX compiler ABI info - done
# -- Check for working CXX compiler: /usr/bin/c++ - skipped
# -- Detecting CXX compile features
# -- Detecting CXX compile features - done
# -- Setting build type to 'Release'
# --   NOTE: You can choose a build type by calling cmake with one of:
# --     -DCMAKE_BUILD_TYPE=Release   -- full optimizations
# --     -DCMAKE_BUILD_TYPE=Debug     -- better debugging experience in gdb
# --     -DCMAKE_BUILD_TYPE=RelASan   -- full optimizations plus address and undefined-behavior sanitizers
# --     -DCMAKE_BUILD_TYPE=DebugASan -- debug plus sanitizers
# -- Found Doxygen: /usr/bin/doxygen (found version "1.9.8") found components: doxygen dot
# -- Found clang-tidy version 2
# -- Found cppcheck
# -- Configuring done (1.0s)
# -- Generating done (0.2s)
# -- Build files have been written to: /home/cs144/computer_network/sponge/build


make -j$(nproc)

# fix all test scripts at once (recommended)
chmod +x ../tests/*.sh


# before implementing the get_URL function
cs144@cs144vm:~/computer_network/sponge/build$ make check_webget
# Testing webget...
# Test project /home/cs144/computer_network/sponge/build
#     Start 31: t_webget
# 1/1 Test #31: t_webget .........................***Failed    0.03 sec
# Function called: get_URL(cs144.keithw.org, /nph-hasher/xyzzy).
# Warning: get_URL() has not been implemented yet.
# ERROR: webget returned output that did not match the test's expectations


# 0% tests passed, 1 tests failed out of 1

# Total Test time (real) =   0.04 sec

# The following tests FAILED:
#          31 - t_webget (Failed)
# Errors while running CTest
# make[3]: *** [CMakeFiles/check_webget.dir/build.make:71: CMakeFiles/check_webget] Error 8
# make[2]: *** [CMakeFiles/Makefile2:7376: CMakeFiles/check_webget.dir/all] Error 2
# make[1]: *** [CMakeFiles/Makefile2:7383: CMakeFiles/check_webget.dir/rule] Error 2
# make: *** [Makefile:2747: check_webget] Error 2



#  manuly run ./apps/webget excutable application under build directory
cs144@cs144vm:~/computer_network/sponge/build$ ./apps/webget cs144.keithw.org /nph-hasher/xyzzy
# HTTP/1.1 200 OK
# Content-type: text/plain

# 7SmXqWkrLKzVBCEalbSPqBcvs11Pw263K7x4Wv3JckI

# manuly run ./tests/webget_t.sh
cs144@cs144vm:~/computer_network/sponge/build$ ../tests/webget_t.sh 
# HTTP/1.1 200 OK
# Content-type: text/plain

# 7SmXqWkrLKzVBCEalbSPqBcvs11Pw263K7x4Wv3JckI



cs144@cs144vm:~/computer_network/sponge/build$ make check_webget
# Testing webget...
# Test project /home/cs144/computer_network/sponge/build
#     Start 31: t_webget
# 1/1 Test #31: t_webget .........................   Passed    1.41 sec

# 100% tests passed, 0 tests failed out of 1

# Total Test time (real) =   1.42 sec
# Built target check_webget

```

# 4 An in-memory reliable byte stream

By now, you've seen how the abstraction of a reliable byte stream can be useful in communicating across the Internet, even though the Internet itself only provides the service of "best-effort" (unreliable) datagrams.
To finish off this week's lab, you will implement, in memory on a single computer, an object that provides this abstraction. (You may have done something similar in CS 110.) 

Bytes are written on the "input" side and can be read, in the same sequence, from the "output" side. 
- The byte stream is finite: the writer can end the input, and then no more bytes can be written. 
- When the reader has read to the end of the stream, it will reach "`EOF`" (end of file) and no more bytes can be read.

Your byte stream will also be `flow-controlled` to limit its memory consumption at any given time. 
- The object is initialized with a particular "capacity": the maximum number of bytes it's willing to store in its own memory at any given point. 
- The byte stream will limit the writer in how much it can write at any given moment, to make sure that the stream doesn't exceed its storage capacity. As the reader reads bytes and drains them from the stream, the writer is allowed to write more. 
- Your byte stream is for use in a single thread you don't have to worry about concurrent writers/readers, locking, or race conditions.

To be clear: 
the byte stream is finite, but it can be almost arbitrarily long before the writer ends the input and finishes the stream. Your implementation must be able to handle streams that are much longer than the capacity. The capacity limits the number of bytes that are held in memory (written but not yet read) at a given point, but does not limit the length of the stream. An object with a capacity of only one byte could still carry a stream that is terabytes and terabytes long, as long as the writer keeps writing one byte at a time and the reader reads each byte before the writer is allowed to write the next byte.

## Interface
Here's what the interface looks like for the writer:

### Writer
```cpp
// Write a string of bytes into the stream. Write as many as will fit, and return the number of bytes written.
size_t write(const std::string &data);

// Returns the number of additional bytes that the stream has space for
size_t remaining_capacity() const;

// Signal that the byte stream has reached its ending
void end_input();
// Indicate that the stream suffered an error
void set_error();
```

### Reader
And here is the interface for the reader:
```cpp
// Peek at next "len" bytes of the stream
std::string peek_output(const size_t len) const;

// Remove len  bytes from the buffer
void pop_output(const size_t len);

// Read (i.e., copy and then pop) the next "len" bytes of the stream
std::string read(const size_t len);

bool input_ended() const; // `true` if the stream input has ended

bool eof() const; // `true` if the output has reached the ending

bool error() const; // `true` if the stream has suffered an error

size_t buffer_size() const; // the maximum amount that can currently be peeked/read

bool buffer_empty() const; // `true` if the buffer is empty

size_t bytes_written() const; // Total number of bytes written

size_t bytes_read() const; // Total number of bytes popped
```


Please open the `libsponge/byte_stream.hh` and `libsponge/byte_stream.cc` files, and implement an object that provides this interface. As you develop your byte stream implementation, you can run the automated tests with `make check lab0` .

What's next? 
Over the next four weeks, you'll implement a system to provide the same interface, no longer in memory, but instead over an unreliable network. This is the **Transmission Control Protocol**.

## Solution

### test check_lab0
```bash
# build
rm -rf *
cmake ..
make -j8

# test
cs144@cs144vm:~/computer_network/sponge/build$ make check_lab0
[100%] Testing Lab 0...
Test project /home/cs144/computer_network/sponge/build
    Start 26: t_byte_stream_construction
1/9 Test #26: t_byte_stream_construction .......   Passed    0.00 sec
    Start 27: t_byte_stream_one_write
2/9 Test #27: t_byte_stream_one_write ..........***Failed    0.00 sec
Test Failure on expectation:
        Expectation: bytes_written: 3

Failure message:
        The ByteStream should have had bytes_written equal to 3 but instead it was 0

List of steps that executed successfully:
        Initialized with (capacity=15)
             Action: write "cat" to the stream
        Expectation: input_ended: 0
        Expectation: buffer_empty: 0
        Expectation: eof: 0
        Expectation: bytes_read: 0

Exception: The test "write-end-pop" failed

    Start 28: t_byte_stream_two_writes
3/9 Test #28: t_byte_stream_two_writes .........***Failed    0.00 sec
Test Failure on expectation:
        Expectation: bytes_written: 3

Failure message:
        The ByteStream should have had bytes_written equal to 3 but instead it was 0

List of steps that executed successfully:
        Initialized with (capacity=15)
             Action: write "cat" to the stream
        Expectation: input_ended: 0
        Expectation: buffer_empty: 0
        Expectation: eof: 0
        Expectation: bytes_read: 0

Exception: The test "write-write-end-pop-pop" failed

    Start 29: t_byte_stream_capacity
4/9 Test #29: t_byte_stream_capacity ...........***Failed    0.00 sec
Test Failure on expectation:
        Expectation: bytes_written: 2

Failure message:
        The ByteStream should have had bytes_written equal to 2 but instead it was 0

List of steps that executed successfully:
        Initialized with (capacity=2)
             Action: write "cat" to the stream
        Expectation: input_ended: 0
        Expectation: buffer_empty: 0
        Expectation: eof: 0
        Expectation: bytes_read: 0

Exception: The test "overwrite" failed

    Start 30: t_byte_stream_many_writes
5/9 Test #30: t_byte_stream_many_writes ........***Failed    0.00 sec
Test Failure on expectation:
        Expectation: bytes_written: 24

Failure message:
        The ByteStream should have had bytes_written equal to 24 but instead it was 0

List of steps that executed successfully:
        Initialized with (capacity=200000)
             Action: write "sllnelfdoadarsovchjdjzqp" to the stream
        Expectation: input_ended: 0
        Expectation: buffer_empty: 0
        Expectation: eof: 0
        Expectation: bytes_read: 0

Exception: The test "many writes" failed

    Start 31: t_webget
6/9 Test #31: t_webget .........................   Passed    2.88 sec
    Start 48: t_address_dt
7/9 Test #48: t_address_dt .....................   Passed    0.04 sec
    Start 49: t_parser_dt
8/9 Test #49: t_parser_dt ......................   Passed    0.00 sec
    Start 50: t_socket_dt
9/9 Test #50: t_socket_dt ......................   Passed    0.00 sec

56% tests passed, 4 tests failed out of 9

Total Test time (real) =   2.96 sec

The following tests FAILED:
         27 - t_byte_stream_one_write (Failed)
         28 - t_byte_stream_two_writes (Failed)
         29 - t_byte_stream_capacity (Failed)
         30 - t_byte_stream_many_writes (Failed)
Errors while running CTest
make[3]: *** [CMakeFiles/check_lab0.dir/build.make:71: CMakeFiles/check_lab0] Error 8
make[2]: *** [CMakeFiles/Makefile2:7408: CMakeFiles/check_lab0.dir/all] Error 2
make[1]: *** [CMakeFiles/Makefile2:7415: CMakeFiles/check_lab0.dir/rule] Error 2
make: *** [Makefile:2760: check_lab0] Error 2

```


### Debug t_byte_stream_one_write


```bash
cs144@cs144vm:~/computer_network/sponge/build$ make check_lab0
[100%] Testing Lab 0...
Test project /home/cs144/computer_network/sponge/build
    Start 26: t_byte_stream_construction
1/9 Test #26: t_byte_stream_construction .......   Passed    0.01 sec
    Start 27: t_byte_stream_one_write
2/9 Test #27: t_byte_stream_one_write ..........   Passed    0.01 sec
    Start 28: t_byte_stream_two_writes
3/9 Test #28: t_byte_stream_two_writes .........   Passed    0.01 sec
    Start 29: t_byte_stream_capacity
4/9 Test #29: t_byte_stream_capacity ...........   Passed    0.46 sec
    Start 30: t_byte_stream_many_writes
5/9 Test #30: t_byte_stream_many_writes ........   Passed    0.01 sec
    Start 31: t_webget
6/9 Test #31: t_webget .........................   Passed    4.49 sec
    Start 48: t_address_dt
7/9 Test #48: t_address_dt .....................   Passed    0.02 sec
    Start 49: t_parser_dt
8/9 Test #49: t_parser_dt ......................   Passed    0.01 sec
    Start 50: t_socket_dt
9/9 Test #50: t_socket_dt ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 9

Total Test time (real) =   5.05 sec
[100%] Built target check_lab0

```



