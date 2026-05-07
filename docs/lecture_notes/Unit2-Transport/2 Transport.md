# 2.1 TCP service model

![image](../../../images/transport_layer/tcp.png)
![image](../../../images/transport_layer/TCP-handshakes.png)

## The TCP connection 

### 1. TCP connection setup : 3 way handshake

![image](../../../images/transport_layer/TCP-connection-setup-3way-handshake.png)

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
![image](../../../images/transport_layer/tcp-stream-bytes-services.png)


### 3. TCP connection teardown:  4 way handshake

![image](../../../images/transport_layer/TCP-connnection-teardown-4-way-handshake.png)

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

![image](../../../images/transport_layer/TCP-service-model.png)

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


# 2.2 User Datagram Protocol(UDP) Service Model

Here's a comprehensive summary of UDP:

## What is UDP?

**UDP (User Datagram Protocol)** is a simple transport layer protocol used by applications that either:
- Don't need guaranteed delivery (handle retransmissions themselves)
- Just don't need reliable delivery at all

UDP is **much simpler than TCP** - it takes application data, creates a UDP datagram, identifies the destination application, and hands it to the network layer. That's it.

## UDP Datagram Format
![image](../../../images/transport_layer/udp-datagram-format.png)
UDP has only **4 header fields** (compared to TCP's 10+):

1. **Source Port (16 bits)** - Which application the data comes from; used for replies
2. **Destination Port (16 bits)** - Which application to deliver data to at the destination
3. **Length (16 bits)** - Total length of UDP datagram (header + data) in bytes; minimum 8 bytes
4. **Checksum (16 bits)** - Optional in IPv4; if used, covers UDP header + data + parts of IP header (source/dest IP, protocol ID=17)

**Note on checksum:** It violates layering by including IP header info, but this allows UDP to detect datagrams delivered to the wrong destination.

## UDP Port Demultiplexing

![image](../../../images/transport_layer/udp-port-demultiplexing.png)
**How it works:**
1. Process 1 on Host A sends data to Process 1 on Host B (port 177)
2. Data placed in UDP datagram with destination port 177
3. Host A adds its source port for replies
4. Datagram encapsulated in IP datagram and sent
5. Host B extracts UDP datagram and directs data to Process 1 based on port number

**Key insight:** UDP is essentially just a **demultiplexing mechanism** - it divides the stream of datagrams and sends them to the correct process. Some call it the "User **Demultiplexing** Protocol."

## UDP Service Model Properties

![image](../../../images/transport_layer/udp-property.png)
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

![image](../../../images/network_layer/icmp-property.png)

## ICMP Message Types

![image](../../../images/network_layer/icmp-message-types.png)

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
