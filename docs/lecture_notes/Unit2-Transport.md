# 2.1 TCP service model

![image](../../images/transport_layer/tcp.png)
![image](../../images/transport_layer/TCP-handshakes.png)

## The TCP connection 

### 1. TCP connection setup : 3 way handshake

![image](../../images/transport_layer/TCP-connection-setup-3way-handshake.png)

- 1. **SYN message**
     Host A sends a message to B indicating that the TCP layer at A wants to establish a connection with the TCP layer at B.
     The message is called a **SYN message**, which is short for synchronize, because A also sends along the **base number** it will use to identify bytes in the byte stream.
  (If it sends “0” then the numbers will start at zero. If it sends “1,000” then they will start at 1,000.)

- 2. **SYN + ACK**
     B responds with what we call a **SYN + ACK**.
     B signals an ACK because B is acknowledging A’s request and agreeing to establish the communication from A to B.
     The TCP layer at B also sends a SYN back to A to indicate that the TCP layer at B wants to establish a connection with the TCP layer at A.
     It sends a number too, indicating the starting number for the byte stream.
- 3. **ACK**
  A responds with an ACK to indicate that it is accepting the request for communication in the reverse direction. The connection is now setup in both directions. They are now ready to start sending data to each other.

#### Initial Sequence Number (ISN)

Each TCP side chooses its own random ISN during the handshake to prevent old data from being confused with new data (helps prevent replay attacks).

- In SYN segment: Sequence Number = ISN, no data yet.
- In ACK segment: Acknowledgment Number = ISN + 1.

### 2. TCP stream bytes services
![image](../../images/transport_layer/tcp-stream-bytes-services.png)


### 3. TCP connection teardown:  4 way handshake

![image](../../images/transport_layer/TCP-connnection-teardown-4-way-handshake.png)

When A and B have finished sending data to each other, they need to close the connection. We say they “teardown” the connection, which means they tell each other they are closing the connection and both ends can clean up the state associated with the state machine.

1. **FIN message**
   The TCP layer at Host A can close the connection by sending a **FIN message**, which is short for FINISH.
2. **ACK**
   Host B acknowledges that A no longer has data to send and stops looking for new data from A. This closes down the data stream from A to B. But B might still have new data to send to A and is not ready to close down the channel from B to A. So the message from B to A carrying the **ACK** can also carry new data from B to A. B can keep sending new data to A as long as it needs to.
3. **FIN**
   Sometime later B finishes sending data to A, and now sends its own FIN to tell A they can close the connection.
4. **ACK**
   Host A replies by sending an **ACK** to acknowledge that the connection is now closed. Because both directions have finished, the connection is now fully closed and the state can be safely removed.

## TCP service model Properties

![image](../../images/transport_layer/TCP-service-model.png)

### 1. Steam of Bytes

it provides a reliable stream of bytes between two applications.


#### What is a "Stream of Bytes" in TCP?

**Stream of bytes** means TCP provides an abstraction where data flows like a continuous stream of water, rather than discrete chunks:

**Key characteristics:**

1. **No message boundaries**: Unlike sending individual letters, TCP delivers data as a continuous flow. If you send "Hello" and then "World", the receiver might get "HelloWorld" all at once, or "Hel" then "loWorld" - TCP doesn't preserve where one write ends and another begins.

2. **In-order delivery**: Bytes arrive in exactly the same order they were sent - byte 1, byte 2, byte 3, etc.

3. **Reliable**: All bytes are guaranteed to arrive (or the connection breaks). No data loss.

4. **Bidirectional**: Both sides can send streams to each other simultaneously.

**Example:**
```
Sender writes: "ABC" then "DEF" then "GHI"
Receiver might read: "ABCDEFGHI" (all at once)
Or: "AB" then "CDEFG" then "HI" (arbitrary chunks)
But never: "ACB" or "DEF" before "ABC" (always in order)
```

#### Relation to Streaming API Responses

When you see "streaming API responses" (like ChatGPT or Claude streaming responses), this is **built on top of** TCP's byte stream, but it's a **different concept at the application layer**:

**TCP layer (transport):**
- Provides the underlying reliable byte stream connection
- Doesn't know or care what the bytes mean
- Just ensures bytes flow reliably and in order

**HTTP/Application layer (streaming response):**
- Uses TCP's byte stream as the foundation
- **Adds structure** on top: sends the response incrementally as it's generated
- The application chooses to send data in chunks rather than waiting for everything to complete

**Example flow:**

```
Application: Generate text token by token
    ↓
HTTP: Send each token as it's ready (chunked transfer encoding)
    ↓
TCP: Break into segments, ensure reliable delivery
    ↓
IP: Route packets to destination
    ↓
TCP: Reassemble segments into byte stream
    ↓
HTTP: Parse chunks as they arrive
    ↓
Application: Display tokens to user in real-time
```

#### Key Difference

- **TCP stream**: Low-level transport mechanism - just moves bytes reliably
- **Streaming API response**: High-level application pattern - chooses to send data incrementally for better user experience (showing results as they're generated rather than waiting for completion)

**Both use the word "stream" but mean different things:**
- TCP: "continuous flow of bytes without boundaries"
- Streaming API: "sending response data progressively over time"

The streaming API response uses TCP's byte stream underneath, but adds application-level meaning about when and how to send data chunks to create the real-time experience you see in chat interfaces.

Does this help clarify the distinction?



### 2. Reliable delivery

It uses four mechanisms to make the communication reliable – in other words, to make sure the data is correctly delivered.

1. When a TCP layer receives data, it sends an acknowledgment back to the sender to let it know the data arrived correctly.

2. Checksums detect corrupted data.
   The TCP header carries a checksum covering the header and the data inside the segment. The checksum is there to detect if the segment is corrupted along the way, for example by a bit-error on the wire or by a memory fault inside a router.

3. Sequence numbers detect missing data.
   Every segment’s header carries the sequence number - in the stream of bytes – of the first byte in the segment.
   For example, if the two sides agree that the sequence numbers start at 1,000 then the first segment will have a sequence number of 1,000. If the segment carries 500 bytes of data then the next segment will carry the sequence number 1,500. If a segment gets lost, then the sequence number will be incorrect and the TCP layer knows some data is missing. It is possible it will show up later – perhaps it took a longer path – or it might have gone missing, in which case the sender will need to resend the data.

4. Flow-control prevents overrunning the receiver.
   If Host A is much faster than Host B then it’s possible for Host A to overwhelm Host B by sending data so fast that Host B can’t keep up. TCP prevents this from happening using something we call flow-control. In TCP, the receiver keeps telling the sender if it can keep sending; specifically, it tells the sender how much room it has in its buffers to accept new data. If Host B is falling behind, the space drops – possibly all the way to zero. When it has more room, it tells A and it can send more data.

### 3. In-sequence

TCP delivers data to the application in the right sequence; in other words, whatever sequence the data was delivered from the application to TCP at host A, this is the same order in which it is sent from TCP to the application at B. If segments arrive out of order, the TCP layer re-sequences them to the correct order, using the sequence number.

- TCP is a byte-oriented protocol, not message-oriented.
- Each byte of data sent over TCP is assigned a sequence number.
- The sequence number in the TCP header indicates "this is the number of the first byte in this segment".

### 4. Congestion Control

TCP provides a service to the whole network by controlling congestion. TCP tries to divide up the network capacity equally among all the TCP connections using the network. The congestion control mechanisms in TCP are very complicated and we’ll devote the whole of Unit 4 to studying congestion control.

## TCP segment format

![image](../../images/transport_layer/TCP-segement-format.png)

The TCP Segment header is much longer and more complicated than, say the IP and Ethernet headers. That is because a TCP connection is reliable – In order to make the communication reliable, the two ends of the connection need to exchange more information so they know which bytes have arrived, which are missing, and the status of the connection.
Here is a quick summary of the most important fields in the TCP header. You don’t need to remember the layout of the header, but you should learn what each field does. If you need a reference, I’d recommend Wikipedia or the Kurose and Ross textbook.

- **Destination port**
  The Destination port tells the TCP layer which application the bytes should be delivered to at the other end. When a new connection starts up, the application tells TCP which service to open a connection with.
  For example, if TCP is carrying **web data**, it uses port 80, which is the port number for TCP. You’ll learn more about port numbers later, but if you are curious, you can look up the well known port numbers at the IANA website. Search for IANA port numbers. You’ll find thousands of port numbers defined for different well known services.
  For example, when we open a connection to an **ssh server**, we use destination port 22.
  For **smtp (the simple mail transfer protocol)** we use port 23. Using a well known port number lets Host B identify the application it should establish the connection with.

- **Source port**:
  The Source port tells the TCP layer at the other end which port it should use to send data back again.
  In our example, when Host B replies to Host A, it should place Host A’s source port number in the destination port field, so that Host A’s TCP layer can deliver the data to the correct application. When a new connection starts, the initiator of the connection – in our case Host A – generates a unique source port number, so differentiate the connection from any other connections between Host A and B to the same service.

- **Sequence number**
  The Sequence number indicates the position in the byte stream of the first byte in the TCP Data field.

  A TCP sequence number is a 32-bit number that uniquely identifies the position of the first byte of data in a TCP segment within the overall data stream.

  - The sequence number in a segment from A to B includes the sequence number of the first byte, offset by the initial sequence number.
  - The acknowledgment sequence number in the segment from B back to A tells us which byte B is expecting next, offset by A’s initial sequence number.


  For example:
  if the Initial Sequence number(ISN) (chosen by the sender) is 1,000 and this is the first segment, then the Sequence number is 1,000. If the sender want to send segment is 500 bytes long, then the sequence number in the next segment will be 1,500 and so on.

  | Segement | Sequence number | Payload Size | Next Sequence number |
  | -------- | --------------- | ------------ | -------------------- |
  | 1        | 1,000           | 500          | 1,500                |
  | 2        | 1,500           | 100          | 1,600                |
  | 3        | 1,600           | 500          | 2,100                |

  Why do we need it?
    - Ordering: Helps the receiver put segments back in the correct order.
    - Reliability: Allows the receiver to detect missing segments and ask for retransmission (via ACKs).
    - Flow Control: In combination with acknowledgment numbers and window sizes, it helps manage data flow.

- **Acknowledgment sequence number**
  The Acknowledgment sequence number tells the other end which byte we are expecting next. It also says that we have successfully received every byte up until the one before this byte number.

  For example, if the Acknowledgment Sequence number is 751, it means we have received every byte up to and including byte 750. Notice that there are sequence numbers for both directions in every segment. This way, TCP piggybacks acknowledgments on the data segments traveling in the other direction.

- **checksum**
  The 16 bit checksum is calculated over the entire header and data, and helps the receiver detect corrupt data. For example, bit errors on the wire, or a faulty memory in a router. You’ll learn more about error detection and checksums in a later video.

- **Header Length**
  The Header Length field tells us how long the TCP header is.

- **TCP Options fields**
  The TCP Options fields are, well, optional. They carry extra, new header fields that were thought of and added after the TCP standard was created. The Header Length field tells us how many option fields are present. Usually there are none.

- **Flags**
  Finally, there are a bunch of Flags used to signal information from one end of the connection to the other.
  - The **ACK** flag tells us that the Acknowledgement sequence number is valid and we are acknowledging all of the data up until this point.
  - The **SYN flag** tells us that we are signalling a synchronize, which is part of the 3way handshake to set up the connection.
  - the **FIN flag** signals the closing of one direction of the connection.
  - the **PSH flag** tells the TCP layer at the other end to deliver the data immediately upon arrival, rather than wait for more data. This is useful for short segments carrying time critical data, such as a key stroke. We don’t want the TCP layer to wait to accumulate many keystrokes before delivering them to the application.

### TCP connection Unique ID

![image](../../images/transport_layer/tcp-connection-unique-id.png)

A TCP connection is uniquely identified by five pieces of information in the TCP and IP headers.

#### The Five-Tuple Identifier

A TCP connection is **uniquely identified by five pieces of information** from the TCP and IP headers:

1. **Source IP Address** - Identifies the source host
2. **Destination IP Address** - Identifies the destination host  
3. **Protocol ID** - Set to TCP (value "6")
4. **Source Port** - Identifies the application process on the source host
5. **Destination Port** - Identifies the application process on the destination host

**Together, these 5 fields uniquely identify the TCP connection Internet-wide at any instant.**

#### Ensuring Uniqueness: Source Port Selection

**Problem:** Host A must pick a unique source port to avoid conflicts with existing connections to the same destination.

**Solution:**
1. **Increments the source port number** for every new connection
   - The field is 16 bits (65,536 possible values)
   - Takes 64K=2^16 new connections before wrapping around


**Potential problem:** If Host A creates many connections rapidly, the port number might wrap around and reuse the same 5-tuple. Old TCP segments stuck in the network (in router buffers or temporary loops) could get confused with the new connection.

2. TCP picks ISN to avoid overlap with previous connections with same ID
![image](../../images/transport_layer/sequence-numbers.png)
**Solution: Random Initial Sequence Numbers (ISN)**
- Each TCP connection initializes with a **random initial sequence number**
- When Host A initiates the connection to B:
  - A includes its ISN for the byte stream A→B
- When B replies:
  - B includes its own ISN for the byte stream B→A
- This significantly reduces chances of confusion between old and new connections


**In segments from A to B:**
- **Sequence number** = position of first byte in this segment, offset by A's ISN

**In segments from B to A:**
- **Acknowledgment number** = next byte B expects to receive, offset by A's ISN

This combination of the 5-tuple plus random ISNs ensures reliable, unambiguous connection identification across the entire Internet.







### TCP Port Demultiplexing

![image](../../images/transport_layer/tcp-port-demultiplexing.png)

----


# 2.2 User Datagram Protocol(UDP) Service Model

Here's a comprehensive summary of UDP:

## What is UDP?

**UDP (User Datagram Protocol)** is a simple transport layer protocol used by applications that either:
- Don't need guaranteed delivery (handle retransmissions themselves)
- Just don't need reliable delivery at all

UDP is **much simpler than TCP** - it takes application data, creates a UDP datagram, identifies the destination application, and hands it to the network layer. That's it.

## UDP Datagram Format
![image](../../images/transport_layer/udp-datagram-format.png)
UDP has only **4 header fields** (compared to TCP's 10+):

1. **Source Port (16 bits)** - Which application the data comes from; used for replies
2. **Destination Port (16 bits)** - Which application to deliver data to at the destination
3. **Length (16 bits)** - Total length of UDP datagram (header + data) in bytes; minimum 8 bytes
4. **Checksum (16 bits)** - Optional in IPv4; if used, covers UDP header + data + parts of IP header (source/dest IP, protocol ID=17)

**Note on checksum:** It violates layering by including IP header info, but this allows UDP to detect datagrams delivered to the wrong destination.

## UDP Port Demultiplexing

![image](../../images/transport_layer/udp-port-demultiplexing.png)
**How it works:**
1. Process 1 on Host A sends data to Process 1 on Host B (port 177)
2. Data placed in UDP datagram with destination port 177
3. Host A adds its source port for replies
4. Datagram encapsulated in IP datagram and sent
5. Host B extracts UDP datagram and directs data to Process 1 based on port number

**Key insight:** UDP is essentially just a **demultiplexing mechanism** - it divides the stream of datagrams and sends them to the correct process. Some call it the "User **Demultiplexing** Protocol."

## UDP Service Model Properties

![image](../../images/transport_layer/udp-property.png)
**1. Connectionless Datagram Service**
- No connection establishment needed
- All information is self-contained in each datagram
- Packets may arrive in any order
- Application must resequence if order matters

**2. Unreliable Delivery**
- No acknowledgments sent
- No mechanism to detect missing datagrams
- Doesn't inform application of drops
- Doesn't request retransmissions
- Application can build its own retransmission mechanism if needed (like early NFS did)

**3. Simple wrapper over IP**
- Provides little more than IP service
- Main addition: ability to direct data to correct application via port numbers

## Why Use UDP?

**Applications that use UDP:**

**Request-Response Applications:**
- **DNS (Domain Name System)** - Request fits in one datagram; lightweight and fast; timeout and resend if needed
- **DHCP (Dynamic Host Configuration Protocol)** - Helps new hosts find their IP address; single self-contained request
- **NTP (Network Time Protocol)** - Time synchronization; simple request-response

**Special Requirements:**
- Some real-time streaming audio/video (less common now)
- Applications needing custom retransmission/congestion control
- Most streaming now uses HTTP/TCP instead

## Key Differences: UDP vs TCP

| Feature | TCP | UDP |
|---------|-----|-----|
| **Connection** | Connection-oriented (3-way handshake) | Connectionless |
| **Reliability** | Guaranteed delivery, retransmissions | No guarantees, no retransmissions |
| **Ordering** | In-order delivery guaranteed | No ordering guarantees |
| **Acknowledgments** | ACKs for received data | No ACKs |
| **Flow Control** | Sliding window mechanism | None |
| **Congestion Control** | Built-in mechanisms | None |
| **Header Size** | 10+ fields, larger overhead | 4 fields, minimal overhead |
| **Speed** | Slower (connection setup, reliability overhead) | Faster (no setup, minimal processing) |
| **Use Case** | Reliable byte stream (web, email, file transfer) | Simple messages, real-time apps where speed > reliability |
| **Service Model** | Reliable stream of bytes | Unreliable datagram delivery |
| **State** | Connection state maintained | Stateless |
| **Complexity** | Complex protocol | Very simple protocol |

## Summary

**UDP is deliberately simple:** It's essentially IP + port-based demultiplexing. It trades reliability for simplicity and speed, making it ideal for applications where occasional data loss is acceptable or where the application wants to handle reliability itself in a custom way.

---

# 2.3 The Internet Control Message Protocol (ICMP) Service Model

Making the Network Layer Work

1. The Internet Protocol (IP)
   -The creation of IP datagrams.
   -Hop-by-hop delivery from end to end.

2. Routing Tables

- Algorithms to populate router forwarding tables

3. Internet Control Message Protocol (ICMP)

- Communicates network layer information between end hosts and routers
- Reports error conditions
- Helps us diagnose problems

## ICMP Property

![image](../../images/network_layer/icmp-property.png)

## ICMP Message Types

![image](../../images/network_layer/icmp-message-types.png)

## How ping uses ICMP

The ping command is a diagnostic tool that uses the ICMP protocol to test whether a network host is reachable and how long it takes for packets to travel between two hosts.

ping sends **ICMP Echo Request messages** and waits for **ICMP Echo Reply messages**.

```bash
# gitbash
michael@DESKTOP-2KLOSPO MINGW64 /e/projects/computer_network (main)
$ ping www.stanford.edu

正在 Ping pantheon-systems.map.fastly.net [146.75.114.133] 具有 32 字节的数据:
来自 146.75.114.133 的回复: 字节=32 时间=72ms TTL=52
来自 146.75.114.133 的回复: 字节=32 时间=73ms TTL=52
来自 146.75.114.133 的回复: 字节=32 时间=73ms TTL=52
来自 146.75.114.133 的回复: 字节=32 时间=72ms TTL=52

146.75.114.133 的 Ping 统计信息:
    数据包: 已发送 = 4，已接收 = 4，丢失 = 0 (0% 丢失)，
往返行程的估计时间(以毫秒为单位):
    最短 = 72ms，最长 = 73ms，平均 = 72ms

# wsl bash
michael@DESKTOP-2KLOSPO:/mnt/e/projects/operating_system/xv6-labs-2020$ ping www.stanford.edu
PING pantheon-systems.map.fastly.net (146.75.114.133) 56(84) bytes of data.
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=1 ttl=51 time=72.7 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=2 ttl=51 time=73.4 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=3 ttl=51 time=73.3 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=4 ttl=51 time=74.8 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=5 ttl=51 time=72.8 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=6 ttl=51 time=73.0 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=7 ttl=51 time=73.4 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=8 ttl=51 time=73.5 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=9 ttl=51 time=73.5 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=10 ttl=51 time=72.9 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=11 ttl=51 time=73.5 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=12 ttl=51 time=74.5 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=13 ttl=51 time=73.1 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=14 ttl=51 time=73.0 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=15 ttl=51 time=73.2 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=16 ttl=51 time=74.4 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=17 ttl=51 time=73.8 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=18 ttl=51 time=73.1 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=19 ttl=51 time=72.6 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=20 ttl=51 time=72.9 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=21 ttl=51 time=73.0 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=22 ttl=51 time=72.7 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=23 ttl=51 time=75.2 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=24 ttl=51 time=72.9 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=25 ttl=51 time=74.1 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=26 ttl=51 time=73.0 ms
64 bytes from 146.75.114.133 (146.75.114.133): icmp_seq=27 ttl=51 time=73.2 ms
^C
--- pantheon-systems.map.fastly.net ping statistics ---
27 packets transmitted, 27 received, 0% packet loss, time 26038ms
rtt min/avg/max/mdev = 72.607/73.385/75.205/0.657 ms

```

### Ping process:

- You run ping example.com
- Your system resolves example.com to an IP address.
- It sends an **ICMP Echo Request packet** to that IP.
- The destination host receives it and responds with an **ICMP Echo Reply**.
- ping calculates the **round-trip time (RTT)** and shows you the result.

### What’s inside an ICMP Echo packet?

Each packet includes:

- Type and code (e.g., Type 8 for Echo Request)
- Identifier (usually a process ID)
- Sequence number (for ordering and matching)
- Payload (some data for RTT calculation)

### ICMP Message Types Used by Ping

| ICMP Type | Message Name | Direction      |
| --------- | ------------ | -------------- |
| 8         | Echo Request | Sent by ping   |
| 0         | Echo Reply   | Sent by target |

### Example

```bash
$ ping google.com

PING google.com (142.250.4.78): 56 data bytes
64 bytes from 142.250.4.78: icmp_seq=0 ttl=115 time=14.3 ms
64 bytes from 142.250.4.78: icmp_seq=1 ttl=115 time=13.8 ms
```

- icmp_seq: matches the ICMP sequence number
- ttl: time-to-live (how many hops left)
- time: round-trip time in milliseconds

### What if no reply?

```bash
michael@DESKTOP-2KLOSPO MINGW64 /e/projects/computer_network (main)

$ ping www.google.com

正在 Ping www.google.com [31.13.73.9] 具有 32 字节的数据:
请求超时。
请求超时。
请求超时。
请求超时。
31.13.73.9 的 Ping 统计信息:
    数据包: 已发送 = 4，已接收 = 0，丢失 = 4 (100% 丢失)，
```
If there is no response, ping will show Request timeout, which could mean:
- Host is unreachable
- ICMP is blocked by a firewall
- The target doesn't respond to ICMP Echo Requests

## How traceroute uses ICMP

traceroute shows the path that packets take from your computer to a destination host, hop-by-hop.

Each “hop” is typically a router that forwards the packet toward the destination. traceroute figures this out by manipulating the TTL (Time-To-Live) field in the IP header.

### Example

```bash
$ traceroute google.com

1  192.168.1.1 (192.168.1.1)  1.123 ms  1.032 ms  0.991 ms
2  10.10.0.1 (10.10.0.1)     5.237 ms  5.412 ms  5.389 ms
3  172.217.0.1 (172.217.0.1) 15.332 ms 14.988 ms 15.102 ms
...

```


### How Does It Work?

Here’s the high-level mechanism:

1. traceroute sends packets with a TTL = 1, then TTL = 2, and so on.
2. When a router receives a packet:

- It decrements the TTL.
- If TTL becomes 0, it discards the packet and sends back an ICMP "Time Exceeded" (Type 11) message to the sender.

3. traceroute uses the returned ICMP messages to:

- Learn the IP address of each hop
- Measure the round-trip time to each hop

4. When the packet finally reaches the destination, the destination host responds with either:

- An ICMP "Port Unreachable" (if UDP is used),
- Or an ICMP Echo Reply (if ICMP is used, like on Windows),
- Or nothing at all if ICMP is blocked.



### ICMP Messages Used Internally

| ICMP Type | Meaning                 | When It's Seen                              |
| --------- | ----------------------- | ------------------------------------------- |
| 11        | Time Exceeded           | When TTL expires at a router                |
| 3         | Destination Unreachable | When packet reaches host but port is closed |
| 0         | Echo Reply              | When using ICMP Echo (like on Windows)      |

---

# 2.4 The End-to-End Principle
Here's a comprehensive summary of the End-to-End Principle:

## The End-to-End Principle (Two Versions)

The end-to-end principle is fundamental to Internet design and exists in two forms: the **basic principle** (about correctness) and the **strong principle** (about design philosophy).

---

## 1. The Basic End-to-End Principle (Correctness)

**Definition (Saltzer, Reed, and Clark, 1984):**
> "The function in question can completely and correctly be implemented only with the knowledge and help of the application standing at the end points of the communication system. Therefore, providing that questioned function as a feature of the communication system itself is not possible."

**Key Insight:** For a system to work **correctly**, the endpoints must be responsible. The network can help, but you can't depend on it alone.

### Example 1: File Transfer Corruption

**Scenario:** Transfer a file from computer A to B through intermediate routers C, D, E.

**What could go wrong:**
- Each link has error detection (detects transmission errors)
- MIT programmers assumed: "If no link corrupts data, the file arrives correctly"
- **They were wrong** - they lost source code because of this assumption

**What actually happened:**
- Computer D had buggy memory
- D received correct packets (passed link checks)
- D stored packets in memory → **bits flipped in memory**
- D forwarded corrupted packets
- Link E's check passed (designed for transmission errors, not storage errors)
- File arrived corrupted at destination

**The Solution:** 
- **End-to-end check required** - source includes error detection info with entire file
- Destination checks the complete file after reassembly
- Only this can guarantee correctness
- Example: BitTorrent uses TCP for chunks, then performs hash checks on each completed chunk

### Example 2: TCP Reliability

**Reality:** TCP provides reliable byte stream, but reliability isn't perfect
- Bugs in TCP stack possible
- Errors can creep in somewhere
- **You still need end-to-end verification** even when using TCP

**Performance Enhancement (allowed by the principle):**
- Wireless links have poor reliability (50-80% vs. wired 99.999%)
- Wireless link layers add **link-layer retransmissions** and acknowledgments
- Boosts reliability from 80% to 99%+
- Greatly improves TCP performance
- **But:** TCP would still work correctly without this help (just slower)

**Key takeaway:** "You can do stuff in the middle to help as performance improvements, but if you don't rely on end-to-end then at some point it will break."

---

## 2. The Strong End-to-End Principle (Design Philosophy)

**Definition (RFC 1958, "Architectural Principles of the Internet"):**
> "The network's job is to transmit datagrams as efficiently and flexibly as possible. Everything else should be done at the fringes..."

**Difference from Basic Principle:**
- **Basic:** Must implement end-to-end for correctness, CAN add middle help for performance
- **Strong:** Should ONLY implement at endpoints, do NOT implement in the middle

### Why the Strong Principle?

**Reasoning: Flexibility and Simplicity**

When the network implements functionality to "help" endpoints:
- It **assumes what endpoints do**
- This creates **dependency and calcification**

**Example: WiFi Link Layer Retransmissions**

**The tradeoff:**
- WiFi retransmits to improve reliability (helps TCP)
- But this increases latency
- Assumes: reliability is worth the latency cost

**The problem:**
- Some protocols DON'T need reliability (e.g., real-time video/audio)
- They'd rather send NEW packets than retry old ones
- But they're **stuck with WiFi's behavior** - can't opt out
- This **impedes innovation**

**The calcification effect:**
1. WiFi (link layer) assumes certain network/transport layer behavior
2. New transport protocols must assume how WiFi behaves to perform well
3. Layers become interdependent
4. Network design becomes **calcified - hard to change**

---

## Why Doesn't the Network Do More?

**The network COULD do many things:**
- **Compression** - reduce file size 10x for text
- **Request optimization** - combine multiple file transfers
- **Smart routing** - fetch from closer server C instead of distant A
- **Automatic security** - encrypt data
- **Mobility support** - routes update as devices move
- **Connection migration** - move Skype call from phone to laptop

**Why it doesn't:**
- Only endpoints have complete knowledge to do these things **correctly**
- Network can't know application requirements
- Adding features creates assumptions that limit flexibility
- Makes the network harder to evolve

---

## The Tension

**Short-term vs. Long-term:**

| Perspective | Approach | Result |
|-------------|----------|--------|
| **Short-term** (network engineers/operators) | Add optimizations in the middle | Better performance now |
| **Long-term** (architects/researchers) | Follow strong end-to-end | Easier to evolve and innovate |

**Reality:** Over time, networks perform better and better but become harder and harder to change.

---

## Summary

**Basic End-to-End Principle:**
- For **correctness**, functionality must be implemented end-to-end
- Network can provide incomplete versions as **performance enhancements**
- But you must NOT depend on the network alone

**Strong End-to-End Principle:**
- Keep the network **simple** - just transmit datagrams efficiently
- Implement everything else at the **endpoints/fringes**
- Preserves **flexibility and evolvability**
- Prevents **calcification** of network design

**The trade-off:** Performance improvements now vs. ability to innovate later.

---
# 2.5 Error Detection:

![image](../../images/transport_layer/error-detection.png)

- Ethernet appends a cyclic redundancy code
- Transport Layer Security appends a message authentication code
- IP prepends a checksum, which it places in the IP header.

  TLS and Ethernet have a footer, protocol information which follows the payload, which is where they put the CRC and MAC.

### Three Error Detection Schemes

- Checksum adds up values in packet (IP, TCP)
  - Very fast, cheap to compute even in software
  - Not very robust
- Cyclic redundancy code computes remainder of a polynomial (Ethernet)
  - More expensive than checksum (easy today, easy in hardware)
  - Protects against any 2 bit error, any burst ≤ c bits long, any odd number of errors
- Message authentication code: cryptographic transformation of data (TLS)
  - Robust to malicious modifications, but not errors
  - If strong, any 2 messages have a 2-c chance of having the same code
- Each layer has its own error detection: end-to-end principle!

## IP Checksum

- IP, UDP, and TCP use one’s complement checksum algorithm:

  - Set checksum field to 0, sum all 16-bit words in packet
  - Add any carry bits back in: 0x8000 + 0x8000 = 0x0001
  - Flip bits (0xc379 becomes 0x3c86), unless 0xffff, then checksum is 0xffff
  - To check:
    It comes naturally from one’s complement logic (sum + flipped sum = all 1s)
    sum whole packet, including checksum, should be 0xffff

- Benefits: fast, easy to compute and check

  - Motivated by earliest software implementations

- Drawbacks: poor error detection
  - Only guarantees detecting a single bit error
  - Can detect other errors, but actual guarantees are both weak and complex

```yaml
# Suppose a TCP segment has this payload:
"hello"

# ASCII codes:
'h' = 0x68
'e' = 0x65
'l' = 0x6C
'l' = 0x6C
'o' = 0x6F

#  if you have a stream of bytes like:
# The Internet is big-endian for checksum purposes: most significant byte comes first.
[0x68, 0x65, 0x6C, 0x6C, 0x6F]

# We group it into 16-bit words (pad last byte if odd number):
# If the total number of bytes is odd, a 0x00 byte is added at the end for alignment.
0x6865
0x6C6C
0x6F00   ← padded with 0

Now we can apply the checksum algorithm using these 16-bit words.
```

```C++
/**
 * In one’s complement,
 * - the negative of a number is represented by flipping all the bits (i.e., bitwise NOT)
 * - Addition rule: Binary add + wrap-around carry
 * - Zero representation: +0 and -0 (both exist)
*/

u_short
cksum(u_short *buf, int count)
{
    register u_long sum = 0;

    while (count--)
    {
        sum += *buf++;
        if (sum & 0xFFFF0000)
        {
            /* carry occurred(overflow), so wrap around */
            sum &= 0xFFFF;
            sum++;
        }
    }
    return ~(sum & 0xFFFF);
}

```

## Cyclic Redundancy Check (CRC)

### Intuition Behind CRC

Think of CRC like a smart remainder you get when dividing your data (viewed as a big binary number) by another fixed binary number (called the generator polynomial). This remainder is appended to the message.

When the receiver gets the message:

1. It divides the whole message (including the CRC bits) by the same generator.
2. If the remainder is zero, the message is assumed to be correct.

If an error has occurred during transmission, the remainder will (almost always) be non-zero, and the receiver knows the data is corrupted.

### How CRC Works (High-Level)

Sender side:

- Treat the data as a binary number.
- Append n zero bits (where n = degree of generator polynomial).
- Divide this by the generator polynomial (using modulo-2 division).
- The remainder is the CRC code.
- Append this CRC to the original data.

Receiver side:

- Divide the entire received message (data + CRC) by the same generator.
- If the remainder is zero, assume no error. Else, error is detected.

### CRC Property

- Cyclic Redundancy Check (CRC): distill n bits of data into c bits, c ≪ n

  - Can’t detect all errors:2^-c chance another packet’s CRC matches

- CRC designed to detect certain forms of errors: stronger than checksum

  - Any message with an odd number of bit errors
  - Any message with 2 bits in error
  - Any message with a single burst of errors ≤ c bits long

- Link layers typically use CRCs
  - Fast to compute in hardware (details in a moment)
  - Can be computed incrementally
  - Good error detection for physical layer burst errors

### Diversion: CRC Mathematical Basis

- Cyclic Redundancy Check (CRC): distill n bits of data into c bits, c << n
- Uses polynomial long division
  - Consider the message M a polynomial with coefficients 0 or 1 (pad with c zeroes)
    - E.g., M = 10011101 = x^7 + x^4 + x^3 + x^2 + 1
  - Use a generator polynomial G of degree c also with coefficients 0 or 1 (binary polynomia)
    - Pad first term (always 1) for frustrating historical reasons
    - E.g. G = 1011 = x^4 + x^3 + x + 1
    - USB (CRC-16) = 0x8005 = x^16 + x^15 + x^2 + 1
  - Divide M by G, the remainder is the CRC: pick G carefully!
- Append CRC to message M: M’ = M + CRC
  - Long division of M’ with G has a remainder of 0

## MAC

A MAC is like a secure fingerprint attached to a message.
It tells the receiver:

- “This message has not been changed”
- “It was created by someone who knows the secret key”

### How it works:

1. Sender and receiver share a secret key.
2. The sender computes a MAC tag using:

- the message content
- the secret key

3. The MAC tag is sent with the message.

4. The receiver:

- recomputes the MAC using the same secret key and received message
- compares it with the tag that came along
- if they match → message is valid!

### MAC Property

- Message Authentication Code (MAC)
  - Not to be confused with Media Access Control (MAC)!
- Uses cryptography to generate c = MAC(M, s), |c| ≪ |M|
  - Using M and secret s, can verify c = MAC(M, s)
  - If you don’t have s, very very hard to generate c
  - Very very hard to generate an M whose MAC is c
  - M + c means the other person probably has the secret (or they’re replayed!)
- Cryptographically strong MAC means flipping one bit of M causes every bit
  in the new c to be randomly 1 or 0 (no information) - Not as good for error detection as a CRC! - But protects against adversaries

### Common MAC algorithms:

- HMAC (based on hash functions like SHA-256)
- CMAC (based on block ciphers like AES)

# 2.6 Finite State Machines

A Finite State Machine is a model of computation made up of:

- A finite number of states
- Transitions between those states triggered by events

## FSM Example: HTTP Request

So let’s walk through an example, an HTTP request. In practice HTTP requests are a bit more complex than this, there all kinds of options, so for this example we’ll just use a very simple form.
Let’s describe our system this way. 


So here we have a three state system. 
- Idle 
- Page requesting state
- Request pending state

Idele State -> Page requesting state ->  Request pending state

1. Idele State 
   
- In our starting state we are are viewing a page or otherwise idle.

2. Page requesting state
   
- When we want to load a new page, we transition to the page requesting state.
So the event is load new page, and the action is open a connection to the web server. 

3. Idele State:
   
- Once we’ve opened a connection, we’re now in the page requesting state. We’ll transition back to the idle state when the connection closes or when we finish requesting every resource on the page.

4. Request pending state
We need one more state, which describes where we are in requesting a page. On the event of having more resources to request, we take the action of requesting a resource with an HTTP GET. This puts us in the request pending state. 

5. Page requesting state
On the event of receiving the response, our system transitions back to the page requesting state.


On one hand, this is a nice, simple FSM. But if you were to try to implement it, it leaves a lot unsaid. Specifically, we have 4 events in the system: 

- page request
- more requests
- receive response
- connection closed. 
  
So what happens if the connection closes when we’re in the request pending state? 
Or when we receive a page request while in the page requesting state? 
Or receive response while in the idle state?


## FSM Example: TCP Connection

For TCP, this FSM helps describe how a connection behaves when:

- Connecting (3-way handshake)
- Transferring data
- Closing the connection (4-way handshake)
  
[TCP State Diagram](https://en.wikipedia.org/wiki/Transmission_Control_Protocol#Protocol_operation)

![iamge](../../images/transport_layer/fsm-example-tcp-connection.png)

#### Common TCP Lifecycle

```text
Client                                  Server
------                                  ------
CLOSED                                 CLOSED
   |                                      |
   |                                   passive open (listen)
   |                                      |
   |                                      | -> LISTEN
   |                                      |
active open (connect)                     |
   |→ SYN_SENT                            |
   |                                  receiving a SYN message
   |                                      |
   |                                      |-> SYN_RECEIVED
   |                                      |
   |                                  sending SYN + ACK message
receiving SYN+ACK message                 |
   |                                      |
sending ACK message                       |   
   |                                      |
   |                                      |
   |-> ESTABLISHED                        |-> ESTABLISHED
   |                                      |
send/receive data                         |
   |                                      |
close()                                   |
   |→ FIN_WAIT_1                          |
                                    send ACK message
   |→ FIN_WAIT_2                          |
                                    send FIN message
   |                                      |
   |                                      |
   |-> TIME_WAIT                        CLOSED
   |
after 2MSL timeout
   |
CLOSED
```


### TCP Connection State



[TCP Connection state](https://maxnilz.com/docs/004-network/003-tcp-connection-state/)

| State | Endpoint | Description |
|-------|----------|-------------|
| CLOSED | Server and client | No connection state at all |
| LISTEN | Server | Waiting for a connection request from any remote TCP end-point |
| SYN-SENT | Client | Waiting for a matching connection request after having sent a connection request |
| SYN-RECEIVED | Server | Waiting for a confirming connection request acknowledgment after having both received and sent a connection request |
| ESTABLISHED | Server and client | An open connection, data received can be delivered to the user. The normal state for the data transfer phase of the connection |
| FIN-WAIT-1 | Server and client | Waiting for a connection termination request from the remote TCP, or an acknowledgment of the connection termination request previously sent |
| FIN-WAIT-2 | Server and client | Waiting for a connection termination request from the remote TCP |
| CLOSING | Server and client | Waiting for a connection termination request acknowledgment from the remote TCP |
| TIME-WAIT | Server or client | Waiting for enough time to pass to be sure that all remaining packets on the connection have expired |
| CLOSE-WAIT | Server and client | Waiting for a connection termination request from the local user |
| LAST-ACK | Server and client | Waiting for an acknowledgment of the connection termination request previously sent to the remote TCP (which includes an acknowledgment of its connection termination request) |
| CLOSED | Server and client | No connection state at all |




 
So let’s walk through a real example of an FSM, probably the most famous FSM in the Internet. This
diagram here describes the finite state machine of TCP. 
it has 12 states -- but I’ll walk though it bit by bit and you’ll see how it all fits together.

First off, the diagram really has four parts, which we can look at separately. These top 4 states are what
describe how you open a TCP connection. 

- ESTABLISHED state  
  This center state, “ESTABLISHED” is when TCP is sending and receiving data. 
  It’s after the connection has been established but before it’s been closed. 

- CLOSED State  
  These 6 states describe how connections close. 
  This state at the bottom, CLOSED, denotes the connection has closed and the node can forget about it. 

- CLOSING State  
  Note that the top state is also the closed state -- before we open the connection.


### TCP Connection(three-way handshake)

Recall that you start a TCP connection with a three way handshake -- SYN, SYN/ACK, ACK. The client, or
active opener, sends a SYN, synchronization, message to a program listening for connection requests.
When it receives a SYN, it responds with a SYN/ACK, synchronizing and acknowledging the original synchronization. The active opener, on receiving the SYN/ACK, responds with an acknowledgement.

The state diagram here describes how TCP behaves on both sides of the TCP three-way handshake. 

- Server： Listen state 
   
A passive opener is a server. It listens for requests for connections from active openers, clients. So when a program calls listen(), the socket transitions from the orange closed state to the yellow listen state. 

The protocol takes no actions when this happens -- it doesn’t send any messages. If the server calls close on the socket when it’s in the listen state, it transitions immediately to the closed state.

- Client: Close State -> SYN SENT state  
  
Let’s walk through the three way handshake starting with the first step, when a client tries to open a connection and sends a SYN packet to the server.
We can see this first transition for the client side of the connection as this orange arrow from closed to the SYN SENT state. This happens when the client program calls connect -- the event -- and the client sends a SYN message.
So once the first SYN is sent, the client is in the SYN SENT state and the server is in the LISTEN state. 

- Server : Listen state -> SYN RECEIVED state  
  
When the SYN arrives at the server, this leads to this blue transition. You can see the event is receiving a SYN message. The action is to send a SYN/ACK message in response. Now the server is in the SYN RECEIVED state.

- Client : SYN SENT -> ESTABLISHED  
Let’s jump back to the client. Remember, it was in the SYN SENT stage. Now, when it receives the SYN/ACK from the server, it transitions to the ESTABLISHED state. 
Its action is to send an ACK message, the third message of the SYN, SYN/ACK, ACK handshake. Now the client can start sending data to the server.

- Server : SYN RECEIVED -> ESTABLISHED
Finally, let’s go back to the server, which is in the SYN RECEIVED state. When it receives the ACK from the client, it transitions to the ESTABLISHED state and can send data.
There are a couple more transitions during connection opening



# 2.7  Flow Control

- Don’t send more packets than receiver can process
- Receiver gives sender feedback
- Two basic approaches
    - Stop and wait (this video)
    - Sliding window (next video)


Reliable delivery is usually accomplished using a combination of two fundamental mechanisms - acknowledgments and timeouts. 

- **acknowledgments**

An acknowledgment (ACK for short) is a small control frame that a protocol sends back to its peer saying that it has received an earlier frame. By control frame we mean a header without any data, although a protocol can piggyback an ACK on a data frame it just happens to be sending in the opposite direction. The receipt of an acknowledgment indicates to the sender of the original frame that its frame was successfully delivered. If the sender does not receive an acknowledgment after a reasonable amount of time, then it retransmits the original frame. 

- **timeouts**
  
This action of waiting a reasonable amount of time is called a timeout.

## Stop-and-Wait

The idea of stop-and-wait is straightforward: After transmitting one frame, the sender waits for an acknowledgment before transmitting the next frame. If the acknowledgment does not arrive after a certain period of time, the sender times out and retransmits the original frame.

### Exception Examples
![image](../../images/transport_layer/Stop-and-Wait-exception-examples.png)


### Duplicates

In both C and D cases, the sender times out and retransmits the original frame, but the receiver will think that it is the next frame, since it correctly received and acknowledged the first frame. This has the potential to cause duplicate copies of a frame to be delivered.

To address this problem, the header for a stop-and-wait protocol usually includes a 1-bit sequence number—that is, the sequence number can take on the values 0 and 1 and the sequence numbers used for each frame alternate

![image](../../images/transport_layer/Timeline%20for%20stop-and-wait%20with%201-bit%20sequence%20number.png)

- Use 1-bit counter in data and acknowledgments
    - Receiver can tell if new data or duplicate
- Some simplifying assumptions
    - Network does not duplicate packets
    - Packets not delayed multiple timeouts


## Sliding Window

It’s a flow control mechanism that allows multiple packets (segments) to be sent before waiting for an acknowledgment (ACK)—as long as they fall within a window size.

Imagine a window that "slides" over a stream of packets to allow:

- Efficient transmission (avoids waiting after every packet)
- Reliable delivery (enables retransmission on loss)
- Flow control (adapts to receiver's capacity)

### Sender-Side Behavior

![image](../../images/transport_layer/Sliding%20window%20on%20sender.png)
- Maintain 3 variables
  - Send window size (SWS) :  gives the upper bound on the number of outstanding (unacknowledged) frames that the sender can transmit;
  - Last acknowledgment received (LAR):  the sequence number of the last acknowledgment received
  - Last segment sent (LSS): the sequence number of the last segment sent
- Maintain invariant: (LSS - LAR) ≤ SWS



- The sender keeps a window of packets it is allowed to send.
- When an ACK is received, the window slides forward, allowing new data to be sent.
- Every segment has a sequence number (SeqNo)


- Advance LAR on new acknowledgment,  thereby allowing the sender to transmit another frame. 
- Buffer up to SWS segments


### Receiver-Side Behavior

![image](../../images/transport_layer/Sliding%20window%20on%20receiver.png)
- Maintain 3 variables
  - Receive window size (RWS): The receiver advertises a window size (called receive window), which tells the sender how much buffer space is available.
  - Last acceptable segment (LAS)
  - Last segment received (LSR)
- Maintain invariant: (LAS - LSR) ≤ RWS



When a frame with sequence number SeqNum arrives, the receiver takes the following action. 

- If SeqNum <= LFR or SeqNum > LAF, then the frame is outside the receiver’s window and it is discarded. 
- If LFR < SeqNum <= LAF, then the frame is within the receiver’s window and it is accepted. 
  
  Now the receiver needs to decide whether or not to send an ACK. Let SeqNumToAck denote the largest sequence number not yet acknowledged, such that all frames with sequence numbers less than or equal to SeqNumToAck have been received. 
  The receiver acknowledges the receipt of SeqNumToAck, even if higher numbered packets have been received. 
  This acknowledgment is said to be cumulative. It then sets LFR = SeqNumToAck and adjusts LAF = LFR + RWS.

- If received packet is < LAS, send acknowledgment
  - Send cumulative acks: if received 1, 2, 3, 5, acknowledge 3
  - NOTE: TCP acks are next expected data (e.g., ack 4 in above example)

# 2.8 Retransmission Strategies
