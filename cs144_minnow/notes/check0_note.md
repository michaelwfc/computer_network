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

3.1 Let’s get started—setting up the repository on your VM and on GitHub
```bash
git clone https://github.com/cs144/minnow
# not exists, replace with https://github.com/top-mind/cs144-minnow-nju

```