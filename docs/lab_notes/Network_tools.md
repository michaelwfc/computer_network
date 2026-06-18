
# telnet

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


# netcat

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



## Listening and connecting

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





# wireshark

[网络顶级掠食者 Wireshark 抓包从入门到实战](https://www.bilibili.com/video/BV12X6gYUEqA/?spm_id_from=333.337.search-card.all.click&vd_source=b3d4057adb36b9b243dc8d7a6fc41295)

[www.wireshark.org](https://www.wireshark.org/download.html)

1. Request web page from www.cs.brown.edu
   check the IP address

2. Open wireshark
   1. 选择 网卡
   2. filter:
      tcp.port == 80 && ip.addr == 128.148.32.12

      tcp.port == 7897 && ip.addr == 128.148.32.12

3. open browser and request `www.cs.brown.edu` or `curl www.cs.brown.edu`
    issue1. 浏览器refresh 没有效果

![TCP-handshakes](../../images/transport_layer/TCP-handshakes.png)
![TCP-handshakes-wireshark](../../images/transport_layer/TCP-handshakes-wireshark.png)

[ssl-handshakes](https://www.ruanyifeng.com/blog/2014/09/illustration-ssl.html)



# ping

```bash
michael@DESKTOP-2KLOSPO MINGW64 ~
$ ping www.tsinghua.edu.cn

正在 Ping www.tsinghua.edu.cn [101.6.15.66] 具有 32 字节的数据:
来自 101.6.15.66 的回复: 字节=32 时间=44ms TTL=46
来自 101.6.15.66 的回复: 字节=32 时间=45ms TTL=46
来自 101.6.15.66 的回复: 字节=32 时间=45ms TTL=46
来自 101.6.15.66 的回复: 字节=32 时间=44ms TTL=46

101.6.15.66 的 Ping 统计信息:
    数据包: 已发送 = 4，已接收 = 4，丢失 = 0 (0% 丢失)，
往返行程的估计时间(以毫秒为单位):
    最短 = 44ms，最长 = 45ms，平均 = 44ms

```