CS144: Introduction to Computer Networking - Fall 2020

## Lab 4: the summit (TCP in full)

Due: Tuesday, October 27, 5 p.m. Stanford time  
Lab sessions: Wednesdays, Oct. 14 and 21 (6–9 p.m. Stanford time)

Labs 3 & 4 are both due on Oct. 27, but there is a bonus for handing in Lab 3 early.

### 0. Collaboration Policy

The programming assignments must be your own work: You must write all the code you hand in for the programming assignments, except for the code that we give you as part of the assignment. Please do not copy-and-paste code from Stack Overflow, GitHub, or other sources. If you base your own code on examples you find on the Web or elsewhere, cite the URL in a comment in your submitted source code.

Working with others: You may not show your code to anyone else, look at anyone else's code, or look at solutions from previous years. You may discuss the assignments with other students, but do not copy anybody's code. If you discuss an assignment with another student, please name them in a comment in your submitted source code. Please refer to the course administrative handout for more details, and ask on Piazza if anything is unclear.

Piazza: Please feel free to ask questions on Piazza, but please don't post any source code.

### 1. Overview

You have reached the summit.

In Lab 0, you implemented the abstraction of a flow-controlled byte stream (ByteStream). In Labs 1, 2, and 3, you implemented the tools that translate—in both directions—between that abstraction and the one the Internet provides: unreliable datagrams.

Now, in Lab 4, you will make the overarching module, called TCPConnection, that combines your TCPSender and TCPReceiver and handles the global housekeeping for the connection. The connection's TCP segments can be encapsulated into the payloads of user (TCP-in-UDP) or Internet (TCP/IP) datagrams—letting your code talk to billions of other computers on the Internet that speak the same TCP/IP language. Figure 1 again shows the overall design.

![image](../../images/labs/Figure%201-The%20arrangement%20of%20modules%20and%20data.png)

A short note of caution: the TCPConnection mostly just combines the sender and receiver modules that you have implemented in the earlier labs—the TCPConnection itself can be implemented in less than 100 lines of code. If your sender and receiver are robust, this will be a short lab. If not, you may need to spend time debugging, aided by the test failure messages. (We'd discourage you from trying to read the test source code, unless it is as a last resort.) Based on the bimodal experience of students last year, we strongly encourage you to start early and not leave this lab until the night before the deadline.

### 2. Getting started

Your implementation of a TCPConnection will use the same Sponge library that you used in Labs 0–3, with additional classes and tests. We're giving you support code that reads and writes TCP segments into the payloads of user and Internet datagrams. We're also giving you a class (CS144TCPSocket) that wraps your TCPConnection and makes it behave like a normal stream socket, just like the TCPSocket you used to implement webget back in Lab 0. By the end of this lab, you will slightly modify your webget to use your TCP implementation—a CS144TCPSocket instead of a TCPSocket. To get started:

1. Make sure you have committed all your solutions to Lab 3. Please don't modify any files outside the top level of the libsponge directory, or webget.cc. You may have trouble merging the Lab 4 starter code otherwise.

2. While inside the repository for the lab assignments, run git fetch to retrieve the most recent version of the lab assignments.

3. Download the starter code for Lab 3 by running git merge origin/lab4-startercode.

4. Within your build directory, compile the source code: make (you can run, e.g., make -j4 to use four processors when compiling).

5. Outside the build directory, open and start editing the writeups/lab4.md file. This is the template for your lab writeup and will be included in your submission.

### 3. Lab 4: The TCP connection

This week, you'll finish building a working TCP implementation. You've already done most of the work to get there: you've implemented the sender and the receiver. Your job this week is to "wire them up" together into one object (a TCPConnection) and handle some housekeeping tasks that are global to the connection.

Recall: TCP reliably conveys a pair of flow-controlled byte streams, one in each direction. Two parties participate in the TCP connection, and each party acts as both "sender" (of its own outbound byte-stream) and "receiver" (of an inbound byte-stream) at the same time:

```
Ⓐ            TCPConnection                                   TCPConnection             Ⓑ

           TCPSender  --->   "Hello there!"     --->              TCPReceiver


         TCPReceiver         <---       "General Kenobi!"  <---   TCPSender
```

The two parties ("A" and "B" in the above diagram) are called the "**endpoints**" of the connection, or the "**peers**." Your TCPConnection acts as one of the peers. It's responsible for receiving and sending segments, making sure the sender and receiver are informed about and have a chance to contribute to the fields they care about for incoming and outgoing segments.

Here are the basic rules the TCPConnection has to follow:

**Receiving segments.** As shown on Figure 1, the TCPConnection receives TCPSegments from the Internet when its `segment_received` method is called. When this happens, the TCPConnection looks at the segment and:

- if the `RST (reset)` flag is set, sets both the inbound and outbound streams to the error state and kills the connection permanently. Otherwise it...
- gives the segment to the TCPReceiver so it can inspect the fields it cares about on incoming segments: **seqno, SYN, payload, and FIN**.
- if the `ACK` flag is set, tells the TCPSender about the fields it cares about on incoming segments: `ackno` and `window_size`.
- if the incoming segment occupied any sequence numbers, the TCPConnection makes sure that at least one segment is sent in reply, to reflect an update in the ackno and window size.

**Sending segments.** The TCPConnection will send TCPSegments over the Internet:

- any time the TCPSender has pushed a segment onto its outgoing queue, having set the fields it's responsible for on outgoing segments: (**seqno, SYN, payload, and FIN**).
- Before sending the segment, the TCPConnection will ask the TCPReceiver for the fields it's responsible for on outgoing segments: `ackno` and `window_size`. If there is an ackno, it will set the `ACK` flag and the fields in the TCPSegment.

**When time passes.** The TCPConnection has a `tick` method that will be called periodically by the operating system. When this happens, the TCPConnection needs to:

- tell the TCPSender about the passage of time.
- abort the connection, and send a reset segment to the peer (an empty segment with the `RST` flag set), if the number of consecutive retransmissions is more than an upper limit TCPConfig::`MAX_RETX_ATTEMPTS`.
- end the connection cleanly if necessary (please see Section 5).

As a result, the overall structure of each TCPSegment looks like this, with "sender-written" and "receiver-written" fields shown in different colors:


The full interface for the TCPConnection is in the class documentation. Please take some time to read through this. Much of your implementation will involve "wiring up" the public API of the TCPConnection to the appropriate routines in the TCPSender and TCPReceiver. As much as possible, you want to defer any heavy lifting to the sender and receiver that you've already implemented. That said, not everything will be that simple, and there are some subtleties that involve the "global" behavior of the overall connection. The hardest part will be deciding when to fully terminate a TCPConnection and declare it no longer "active."

What follows are some FAQs and details of edge cases that you'll need to handle.

### 4. FAQs and special cases

- **How much code are you expecting?**  
  Overall, we expect the implementation (in tcp_connection.cc) will require about 100–150 lines of code in total. When you're done, the test suite will extensively test your interoperability with your own implementation as well as the Linux kernel's implementation of TCP.

- **How should I get started?**  
  Probably the best way to start is by wiring up some of the "ordinary" methods to the appropriate calls in TCPSender and TCPReceiver. This may include stuff like remaining outbound capacity(), bytes in flight(), and unassembled bytes(). Then you may choose to implement the "writer" methods: connect(), write(), and end_input_stream(). Some of these methods may need to do something to the outbound ByteStream (owned by the TCPSender) and tell the TCPSender about it. You might choose to start running the test suite (`make_check`) before you have fully implemented every method; the test failure messages can give you a clue or a guide about what to tackle next.

- **How does the application read from the inbound stream?**  
  `TCPConnection::inbound_stream() `is implemented in the header file already. You don't have to do anything more to support the application reading.

- **Does the TCPConnection need any fancy data structures or algorithms?**  
  No, it really doesn't. The heavy lifting is all done by the TCPSender and TCPReceiver that you've already implemented. The work here is really just about wiring everything up, and dealing with some lingering connection-wide subtleties that can't easily be factored in to the sender and receiver.

- **How does the TCPConnection actually send a segment?**  
  Similar to the TCPSender—push it on to the segments out queue. As far as your TCPConnection is concerned, consider it sent as soon as you push it on to this queue. Soon the owner will come along and pop it (using the public `segments_out()` accessor method) and really send it.

- **How does the TCPConnection learn about the passage of time?**  
  Similar to the TCPSender—the `tick()` method will be called periodically. Please don't use any other way of telling the time—the tick method is your only access to the passage of time. That keeps things deterministic and testable.

- **What does the TCPConnection do if an incoming segment has the rst flag set?**  
  This flag ("reset") means instant death to the connection. If you receive a segment with rst, you should set the error flag on the inbound and outbound ByteStreams, and any subsequent call to TCPConnection::active() should return false.

- **When should I send a segment with the `RST` flag set?**  
  There are two situations where you'll want to abort the entire connection:
  1. If the sender has sent too many consecutive retransmissions without success (more than TCPConfig::MAX RETX ATTEMPTS, i.e., 8).
  2. If the TCPConnection `destructor` is called while the connection is still active (`active()` returns true).
  Sending a segment with `RST` set has a similar effect to receiving one: the connection is dead and no longer active(), and both ByteStreams should be set to the error state.

- **Wait, but how do I even make a segment that I can set the `RST` flag on? What's the sequence number?**  
  Any outgoing segment needs to have the proper sequence number. You can force the TCPSender to generate an empty segment with the proper sequence number by calling its `send_empty_segment()` method. Or you can make it fill the window (generating segments if it has outstanding information to send, e.g. bytes from the stream or SYN/FIN) by calling its `fill_window()` method.

- **What's the purpose of the `ACK` flag? Isn't there always an ackno?**  
  Almost every TCPSegment has an ackno, and has the `ACK` flag set. The exceptions are just at the very beginning of the connection, before the receiver has anything to acknowledge.  
  On outgoing segments, you'll want to set the ackno and the ack flag whenever possible. That is, whenever the TCPReceiver's `ackno()` method returns a std::optional<WrappingInt32> that has a value, which you can test with `has_value()`.  
  On incoming segments, you'll want to look at the ackno only if the ack field is set. If so, give that ackno (and window size) to the TCPSender.

- **How do I decipher these "state" names (like "stream started" or "stream ongoing")?**  
  Please see the diagrams in the Lab 2 and Lab 3 handouts.  
  We want to emphasize again that the "states" are useful for testing and debugging, but we're not asking you to materialize these states in your code. You don't need to make more state variables to keep track of this. The "state" is just a function of the public interface your modules are already exposing.

- **What window size should I send if the TCPReceiver wants to advertise a window size that's bigger than will fit in the TCPSegment::header().win field?**  
  Send the biggest value you can. You might find the `std::numeric` limits class helpful.

- **When is the TCP connection finally "done"? When can active() return false?**  
  Please see the next section.

- **Where can I read if there are more FAQs after this PDF comes out?**  
  Please check the website (https://cs144.github.io/lab faq.html) and Piazza regularly.

### 5. The end of a TCP connection: consensus takes work

One important function of the TCPConnection is to decide when the TCP connection is fully "done." When this happens, the implementation releases its exclusive claim to a local port number, stops sending acknowledgments in reply to incoming segments, considers the connection to be history, and has its `active()` method return false.

There are two ways a connection can end. 

- In an unclean shutdown, the TCPConnection either sends or receives a segment with the `RST` flag set. In this case, the outbound and inbound ByteStreams should both be in the error state, and active() can return false immediately.

- A clean shutdown is how we get to "done" (active() = false) without an error. This is more complicated, but it's a beautiful thing because it ensures as much as possible that each of the two ByteStreams has been reliably delivered completely to the receiving peer. In the next section (§§5.1), we give the practical upshot for when a clean shutdown happens, so feel free to skip ahead if you like.

Cool, you're still here. Because of the Two Generals Problem, it's impossible to guarantee that both peers can achieve a clean shutdown, but TCP gets pretty close. Here's how. From the perspective of one peer (one TCPConnection, which we'll call the "local" peer), there are four prerequisites to having a clean shutdown in its connection with the "remote" peer:

**Prereq #1** The inbound stream has been fully assembled and has ended.

**Prereq #2** The outbound stream has been ended by the local application and fully sent (including the fact that it ended, i.e. a segment with `FIN`) to the remote peer.

**Prereq #3** The outbound stream has been fully acknowledged by the remote peer.

**Prereq #4** The local TCPConnection is confident that the remote peer can satisfy prerequisite #3. This is the brain-bending part. There are two alternative ways this can happen:

- **Option A: lingering after both streams end.** Prerequisites #1 through #3 are true, and the remote peer seems to have gotten the local peer's acknowledgments of the entire stream. The local peer doesn't know this for sure—TCP doesn't deliver acks reliably (it doesn't ack acks). But the local peer is pretty confident that the remote peer has gotten its acks, because the remote peer doesn't seem to be retransmitting anything, and the local peer has waited a while to make sure.  
  In specific, a connection is done when prereqs #1 through #3 are satisfied and it has been at least 10 times the initial retransmission timeout (cfg.rt timeout) since the local peer has received any segments from the remote peer. This is called "lingering" after both streams finish, to make sure the remote peer isn't trying to retransmit anything that we need to acknowledge. It does mean that a TCPConnection needs to stay alive for a while, keeping an exclusive claim on a local port number and possibly sending acks in response to incoming segments, even after the TCPSender and TCPReceiver are completely done with their jobs and both streams have ended.

- **Option B: passive close.** Prerequisites #1 through #3 are true, and the local peer is 100% certain that the remote peer can satisfy prerequisite #3. How can this be, if TCP doesn't acknowledge acknowledgments? Because the remote peer was the first one to end its stream.

> ?Why does this rule work? This is the brain-bender and you don't need to read further to complete this lab, but it's fun to think about and gets to the deep reasons for the Two Generals Problem and the inherent constraints on reliability across an unreliable network. The reason this works is that after receiving and assembling the remote peer's fin (prerequisite #1), the local peer sent a segment with a greater sequence number than it had ever sent before (at the very least, it had to send its own fin segment to satisfy prerequisite #2), and that segment also had an ackno that acknowledged the remote peer's fin bit. The remote peer acknowledged that segment (to satisfy prerequisite #3), which means that the remote peer must have also seen the local peer's ack of the remote peer's fin. Which guarantees that the remote peer must be able to satisfy its own prerequisite #3. All this means the local peer can satisfy prerequisite #4 without having to linger. Whew! We said it's a brain-bender. Extra credit in your lab writeup: can you find a better way of explaining this?

The bottom line is that if the TCPConnection's inbound stream ends before the TCPConnection has ever sent a fin segment, then the TCPConnection doesn't need to linger after both streams finish.

#### 5.1 The end of a TCP connection (practical summary)

Practically what all this means is that your TCPConnection has a member variable called `_linger_after_streams_finish`, exposed to the testing apparatus through the state() method. The variable starts out true. If the inbound stream ends before the TCPConnection has reached EOF on its outbound stream, this variable needs to be set to false.

At any point where prerequisites #1 through #3 are satisfied, the connection is "done" (and active() should return false) if `_linger_after_streams_finish` is false. Otherwise you need to linger: the connection is only done after enough time (10 × cfg.rt timeout) has elapsed since the last segment was received.

### 6. Testing

In addition to the automated tests, we encourage you to just "play around with" your TCP implementation. Here are some instructions:

Here's an example of how to run manually. You'll need two windows open, both in the sponge/build directory.

In one window, run: `./apps/tcp_udp -l 127.0.0.1 9090`

This runs your TCPConnection as a server (with the segments going inside UDP datagrams), listening on the local address (127.0.0.1), UDP port 9090. You should see:

```
DEBUG: Listening for incoming connection...
```

In another window, run a client: `./apps/tcp_udp 127.0.0.1 9090`

This runs your TCPConnection as a client, connecting to the address that the server is listening on.

In the server window, you should now see:

```
DEBUG: Listening for incoming connection... new connection from 127.0.0.1:39900.
```

And in the client window, you should now see:

```
DEBUG: Connecting to 127.0.0.1:9090... done.
```

Now try typing something in either window (client or server) and hit ENTER. You should see the same text appear in the other window.

Now try ending one of the streams. In the client window, type Ctrl-D. This ends the client's outbound stream. Now the client window should look something like this:

```
DEBUG: Connecting to 127.0.0.1:9090... done.
Hello from the server to the client.
Hello from the client to the server.
DEBUG: Outbound stream to 127.0.0.1:9090 finished (1 byte still in flight).
DEBUG: Outbound stream to 127.0.0.1:9090 has been fully acknowledged.
```

and the server window should look something like this:

```
DEBUG: Listening for incoming connection... new connection from 127.0.0.1:54643.
Hello from the server to the client.
Hello from the client to the server.
DEBUG: Inbound stream from 127.0.0.1:54643 finished cleanly.
```

Finally, in the server window, end the stream in that direction by typing Ctrl-D again.

The server window should now print something like the following and immediately return you to the command line (no lingering):

```
DEBUG: Outbound stream to 127.0.0.1:54643 finished (1 byte still in flight).
DEBUG: Outbound stream to 127.0.0.1:54643 has been fully acknowledged.
DEBUG: TCP connection finished DEBUG: Waiting for clean shutdown... cleanly.
done.
```

and the client window should print something like this:

```
DEBUG: Inbound stream from 127.0.0.1:9090 finished cleanly.
DEBUG: Waiting for lingering segments (e.g. retransmissions of FIN) from peer...
DEBUG: Waiting for clean shutdown...
```

after 10 seconds of lingering, the client window should print the rest and return you to the command line:

```
DEBUG: Waiting for clean shutdown...
DEBUG: TCP connection finished cleanly.
done.
```

If one of these steps is going awry, that suggests a place to look in your termination logic (the decision about when to stop reporting active()=true). Or please feel free to post again here and we can try to help you debug further.

**Testing with a tiny window:** if you're worried about whether your TCPSender is getting stuck when the receiver advertises a zero window case, try running the above commands and use a "-w 1" argument to the tcp_udp program. This will make it use a TCPReceiver capacity of 1, which means that when you type "hello" into one side, only one byte can be sent and then the receiver will advertise a zero window.

Make sure your implementation doesn't get stuck there! It should successfully be able to send an arbitrary length string (e.g. "hello how are you doing") even when the receiver capacity is 1 byte.

### 7. Performance

After you've finished your TCP implementation, and after you are passing all of the tests run by make check, please commit! Then, measure the performance of your system and bring it up to at least 100 megabits per second.

From the build directory, run `./apps/tcp_benchmark`. If all goes well, you'll see output that looks like this:

```
      user@computer:~/sponge/build$ ./apps/tcp benchmark
          CPU-limited throughput                : 1.78 Gbit/s
          CPU-limited throughput with reordering: 1.21 Gbit/s
```

To receive full credit on the lab, your performance needs to be at least "0.10 Gbit/s" (100 megabits per second) on both lines. You may need to profile your code or reason about where it is slow, and you may have to improve your implementation of some of the critical modules (e.g., ByteStream or StreamReassembler) to get to this point.

In your writeup, please report the speed figures you achieved (with and without reordering).

If you would like, you're welcome to try to optimize your code as much as you want, but please do not do this at the expense of other parts of CS144, including other parts of this lab. We won't give extra points for performance that's faster than 100 Mbit/s—any improvements you do beyond this minimum are for your own satisfaction and learning only. If you achieve an implementation that's faster than ours without changing any public interfaces, we would love to learn from you about how you did it.

### 8. webget revisited

Time to take a victory lap! Remember your webget.cc that you wrote in Lab 0? It used a TCP implementation (TCPSocket) provided by the Linux kernel. We'd like you to switch it to use your own TCP implementation without changing anything else. We think that all you'll need to do is:

- Replace `#include "socket.hh"` with `#include "tcp sponge socket.hh"`.
- Replace the TCPSocket type with CS144TCPSocket.
- At the end of your get URL() function, add a call to socket.wait until closed().

> ?Why am I doing this? Normally the Linux kernel takes care of waiting for TCP connections to reach "clean shutdown" (and give up their port reservations) even after user processes have exited. But because your TCP implementation is all in user space, there's nothing else to keep track of the connection state except your program. Adding this call makes the socket wait until your TCPConnection reports active() = false.

Recompile, and run make check webget to confirm that you've gone full-circle: you've written a basic Web fetcher on top of your own complete TCP implementation, and it still successfully talks to a real webserver. If you have trouble, try running the program manually: `./apps/webget cs144.keithw.org /hasher/xyzzy`. You'll get some debugging output on the terminal that may be helpful.

### 9. Development and debugging advice

1. Implement the TCPConnection's public interface (and any private methods or functions you'd like) in the file tcp connection.cc. You may add any private members you like to the TCPConnection class in tcp connection.hh.

2. We are expecting about 100–150 lines of code in total. You won't need any fancy data structures or algorithms;

3. You can test your code (after compiling it) with make check. This will run a fairly comprehensive test suite (159 tests). Many of the tests confirm that your TCP implementation can transfer files error-free with Linux's TCP implementation, or with itself, over various combinations of packet loss and data transfer in each direction.

4. Please re-read the section on "using Git" in the Lab 0 document and in the online FAQs, and remember to keep the code in the Git repository it was distributed in on the master branch. Make small commits, using good commit messages that identify what changed and why.

5. Please work to make your code readable to the CA who will be grading it for style. Use reasonable and clear naming conventions for variables. Use comments to explain complex or subtle pieces of code. Use "defensive programming"—explicitly check preconditions of functions or invariants, and throw an exception if anything is ever wrong. Use modularity in your design—identify common abstractions and behaviors and factor them out when possible. Blocks of repeated code and enormous functions will make it hard to follow your code.

6. Please also keep to the "Modern C++" style described in the Lab 0 document. The cppreference website (https://en.cppreference.com) is a great resource, although you won't need any sophisticated features of C++ to do these labs. (You may sometimes need to use the move() function to pass an object that can't be copied.)

7. If you get a segmentation fault, something is really wrong! We would like you to be writing in a style where you use safe programming practices to make segfaults extremely unusual (no malloc(), no new, no pointers, safety checks that throw exceptions where you are uncertain, etc.). That said, to debug you can configure your build directory with cmake .. -DCMAKE BUILD TYPE=RelASan to enable the compiler's "sanitizers" to detect memory errors and undefined behavior and give you a nice diagnostic about when they occur. You can also use the valgrind tool. You can also configure with cmake .. -DCMAKE BUILD TYPE=Debug and use the GNU debugger (gdb). Remember to use these settings for debugging only—they dramatically slow down both compilation and execution of your programs. The most reliable/foolproof way to revert to "Release" mode is just to blow away the build directory and create a new one.

### 10. Submit

1. In your submission, please only make changes to the .hh and .cc files in the top level of libsponge. Within these files, please feel free to add private members as necessary, but please don't change the public interface of any of the classes.

2. Before handing in any assignment, please run these in order:
   - make format (to normalize the coding style)
   - git status (to check for un-committed changes—if you have any, commit!)
   - make (to make sure the code compiles)
   - make check (to make sure the automated tests pass)

3. Write a report in writeups/lab4.md. This file should be a roughly 20-to-50-line document with no more than 80 characters per line to make it easier to read. The report should contain the following sections:
   - **Program Structure and Design.** Describe the high-level structure and design choices embodied in your code. You do not need to discuss in detail what you inherited from the starter code. Use this as an opportunity to highlight important design aspects and provide greater detail on those areas for your grading TA to understand. You are strongly encouraged to make this writeup as readable as possible by using subheadings and outlines. Please do not simply translate your program into an paragraph of English.
   - **Implementation Challenges.** Describe the parts of code that you found most troublesome and explain why. Reflect on how you overcame those challenges and what helped you finally understand the concept that was giving you trouble. How did you attempt to ensure that your code maintained your assumptions, invariants, and preconditions, and in what ways did you find this easy or difficult? How did you debug and test your code?
   - **Remaining Bugs.** Point out and explain as best you can any bugs (or unhandled edge cases) that remain in the code.

4. Please also fill in the number of hours the assignment took you and any other comments.

5. When ready to submit, please follow the instructions at https://cs144.github.io/submit. Please make sure you have committed everything you intend before submitting.

6. Please let the course staff know ASAP of any problems at the Tuesday-evening lab sessions, or by posting a question on Piazza. Good luck!


# Solution

## TCPConnection: Structure and Interface 

### The Big Picture First
```
Application
    │  write() / inbound_stream()
    ▼
┌─────────────────────────────────────────┐
│              TCPConnection              │
│                                         │
│  ┌─────────────┐   ┌─────────────────┐  │
│  │  TCPSender  │   │  TCPReceiver    │  │
│  │             │   │                 │  │
│  │ seqno       │   │ ackno           │  │
│  │ SYN/FIN     │   │ window_size     │  │
│  │ payload     │   │ reassembly      │  │
│  └─────────────┘   └─────────────────┘  │
│                                         │
│  _segments_out ──────────────────────►  │ (to network)
│                ◄──── segment_received() │ (from network)
└─────────────────────────────────────────┘
```
TCPConnection is the coordinator: it owns both sender and receiver, and is responsible for:

1. Routing incoming segment fields to the right sub-component
2. Merging outgoing segment fields from both sub-components
3. Managing connection-wide lifecycle (RST, linger, clean shutdown)


#### Group 1: Writer Interface (Application → Network)
void connect();
size_t write(const std::string &data);
size_t remaining_outbound_capacity() const;
void end_input_stream();

#### Group 2: Reader Interface (Network → Application)
ByteStream &inbound_stream();

#### Group 3: Diagnostic Accessors (used in tests)
size_t bytes_in_flight() const;
size_t unassembled_bytes() const;
size_t time_since_last_segment_received() const;
TCPState state() const;

#### Group 4: Owner/OS Interface (the core mechanics)
void segment_received(const TCPSegment &seg);
void tick(const size_t ms_since_last_tick);
std::queue<TCPSegment> &segments_out();
bool active() const;

#### Group 5: Constructor and Special Members
explicit TCPConnection(const TCPConfig &cfg);
~TCPConnection();
TCPConnection(TCPConnection &&other) = default;       // move: allowed
TCPConnection(const TCPConnection &other) = delete;   // copy: forbidden

### The Segment Assembly Pipeline
Every time a segment leaves TCPConnection, it goes through this merge step:
Conceptual pipeline for every outgoing segment:
```
TCPSender produces:
  segment.header().seqno  ← set by sender
  segment.header().syn    ← set by sender
  segment.header().fin    ← set by sender
  segment.payload()       ← set by sender
TCPConnection enriches it:
  if (_receiver.ackno().has_value()) {
      segment.header().ack    = true;
      segment.header().ackno  = _receiver.ackno().value();
  }
  segment.header().win = _receiver.window_size();  // clamped to uint16_t max
Then pushed to _segments_out.
```

### TCPConnection is Event-Driven

TCPConnection is not a blocking, sequential state machine like this:
```
// THIS IS NOT HOW IT WORKS
void connect() {
    send SYN
    wait for SYN-ACK     ← WRONG mental model
    send ACK
    // now connected
}
```
Instead, it is reactive. Each method responds to one event and returns immediately:
```
External event          →   Method called
─────────────────────────────────────────
"user wants to connect" →   connect()          → triggers active open (one-time action)
"segment arrived"       →   segment_received() → handles ANY incoming segment, any time
"time passed"           →   tick()             → handles time-based events (retransmit, linger)
```

## Phase1: The Three-Way Handshake


```
Client TCPConnection              Network              Server TCPConnection
        │                                                       │
connect()                                                       │
  _sender.fill_window()                                         │
  → SYN pushed to _segments_out                                 │
        │──────────────── SYN seq=ISN_c ──────────────────────► │
        │                                                segment_received(SYN)
        │                                                  → _receiver gets SYN
        │                                                  → _sender.fill_window()
        │                                                  → SYN+ACK pushed out
        │◄─────────────── SYN+ACK seq=ISN_s, ack=ISN_c+1 ───── │
        │                                                       │
segment_received(SYN+ACK)                                       │
  → _receiver processes SYN                                     │
  → _sender gets ackno=ISN_c+1                                  │
  → segment occupied seqno space                                │
    → must send reply (ACK)                                     │
  → ACK pushed to _segments_out                                 │
        │──────────────── ACK ack=ISN_s+1 ────────────────────► │
        │                                                segment_received(ACK)
        │                                                  → _sender gets ack
        │                                                  → connection open
```


The handshake is not a sequential procedure. It is three separate **events**, each handled by a separate method call, connected only by the state implicitly maintained inside _sender and _receiver.

```

connect()  →  fires SYN, returns immediately
              ↓
              runtime calls segment_received() when SYN-ACK arrives
              ↓
              segment_received() fires ACK, returns immediately
              ↓
              handshake complete
```

### Steps of 3 way handshake

#### Step1: Client connect() 

```c++
void TCPConnection::connect() {
    _sender.fill_window();   // sender sees stream not started → sends SYN
    // push sender's segments to _segments_out (with receiver fields added)
    // return immediately
}
```
That is essentially it. One method call, one SYN segment produced, function returns.
The sender knows to send a SYN because it tracks its own state internally. When fill_window() is called and the sender has never sent anything (next seqno = 0), it produces a SYN segment automatically.

#### Step 2 — Server receives SYN, sends SYN+ACK
On the server side, there is no connect() call. The server simply waits. When the first segment (SYN) arrives:

```c++
segment_received(SYN):
    _receiver.segment_received(seg)   // receiver records ISN, ackno becomes ISN+1
    // ACK flag not set → don't notify sender
    // SYN occupied sequence space → must send a reply
    _sender.fill_window()             // sender sees stream open → sends SYN+ACK
    // enrich with receiver's ackno → ACK flag set
    push to _segments_out
```

Why "Occupied Sequence Space" Means "Must Reply"?
```
Segment type          Occupies sequence space?    Must be ACKed?
─────────────────────────────────────────────────────────────────
SYN                   YES (1 byte)                YES
payload bytes         YES (1 byte each)           YES
FIN                   YES (1 byte)                YES
bare ACK              NO  (0 bytes)               NO
window update only    NO  (0 bytes)               NO
The rule is precise:
length_in_sequence_space() > 0  →  receiver MUST acknowledge it
length_in_sequence_space() == 0 →  no acknowledgment required 
```

The server's SYN+ACK is produced automatically because:

- The receiver now has an ackno to report
- The sender still needs to send its own SYN (it hasn't sent anything yet)
- So the outgoing segment carries both SYN (from sender) and ACK (from receiver)

```
Server receives SYN → must ACK it
Server also needs to send its own SYN

Result: one segment carries both
┌─────────────────────────────────────────┐
│  seqno  = ISN_s                         │
│  SYN    = true    ← sender's job        │
│  ACK    = true    ← receiver's job      │
│  ackno  = ISN_c + 1                     │
│  payload = empty                        │
└─────────────────────────────────────────┘

Sequence space consumed: 1 (SYN only)
Pure ACK? NO — carries SYN
```

Two Types of Segments That Carry ACK
Type 1: Piggybacked ACK    →  ACK rides inside a real data/SYN/FIN segment
Type 2: Pure ACK           →  ACK travels alone, no sequence space consumed

Type 1: Piggybacked ACK
The ACK flag is attached to a segment that already carries real content. This is the efficient case — one segment does two jobs simultaneously.


#### Step 3 — Client receives SYN+ACK, sends ACK

```
Client                                    Server
connect()                                 
  _sender.fill_window()                   
  → SYN pushed to _segments_out           
        │──── SYN seq=ISN_c ────────────► │
        │                                 segment_received(SYN)
        │                                   _receiver.segment_received() gets SYN
        |                                   _sender.ack_received() update ackno, window_size
        │                                   _sender.fill_window() -> ISN_s
        |                                   _sender.send_empty_segment() -> ACK
        │                                   → pushed out SYN+ACK 
        │◄─── SYN+ACK seq=ISN_s ────────── │
        │
segment_received(SYN+ACK)   ← YOU ARE HERE
        │
        │──── ACK ──────────────────────► │

```


The bare ACK in step 3 exists purely because the rule says: 
if an incoming segment occupied sequence space, you must reply with at least one segment. 
The sender has nothing real to send, so it generates an empty segment, and the receiver enriches it with the current ackno. 
That becomes the final ACK of the handshake.


```
SYN+ACK segment:
  syn     = true      ← occupies 1 sequence space
  ack     = true      ← occupies 0 sequence space
  payload = empty     ← occupies 0 sequence space
─────────────────────────────────────────────────
length_in_sequence_space() = 1   (SYN only)
```

Server's perspective:
  "I sent SYN at seqno ISN_s.
   This occupies sequence space.
   Until I receive ACK(ISN_s + 1), I cannot know if client got my SYN.
   I will retransmit SYN+ACK every RTO until ACKed."

If the client never sends the final ACK:
```
t=0ms    server sends SYN+ACK
t=1000ms server retransmits SYN+ACK  (RTO expired)
t=3000ms server retransmits SYN+ACK  (RTO doubled)
t=7000ms server retransmits SYN+ACK
...
t=MAX    server gives up, sends RST
```
The handshake never completes. The connection never opens.




```c++
segment_received(SYN+ACK):
    _receiver.segment_received(seg)   // receiver records server's ISN
    _sender.ack_received(ackno, win)  // sender learns its SYN was acknowledged
    // SYN+ACK occupied sequence space → must send a reply
    // sender has nothing new to send → send_empty_segment()
    // enrich with receiver's ackno → produces bare ACK
    push to _segments_out
```



[1] Receiver processes the SYN
`_receiver.segment_received(seg);`

The SYN+ACK segment carries the server's ISN and has `syn=true`. 


Your TCPReceiver records the server's ISN, marks `_syn_received = true`, and computes:

```c++
ack_absolute = 1 + 0 + 0 = 1   (SYN consumed 1 seqno, no payload yet)
_ackno = wrap(1, _isn)          = ISN_s + 1
```
Now `_receiver.ackno()` returns `ISN_s + 1`. This is crucial — before this call, the receiver had no ackno to report.

[2] Sender processes the ACK

```c++
if (seg.header().ack) {
    _sender.ack_received(seg.header().ackno, seg.header().win);
}
```
The SYN+ACK has `ack=true` and `ackno = ISN_c + 1`. Inside `ack_received()`:

```c++
ackno_absolute = ISN_c + 1

outstanding queue: [SYN segment, seqno=ISN_c, length=1]
seg_end = ISN_c + 1

ackno_absolute(ISN_c+1) >= seg_end(ISN_c+1) → fully ACKed → remove from queue

_last_acked_seqno = ISN_c + 1
_window_size = server's advertised window

acked_something = true → reset timer, reset retransmissions
outstanding queue now empty → stop timer

fill_window() called internally
  → _syn_sent = true, stream is empty, _fin_sent = false
  → no SYN to send, no data, no FIN
  → seg.length_in_sequence_space() == 0 → break
  → nothing pushed to _sender._segments_out
```

So ack_received() produces no new sender segments here. The stream is empty.

[3] If segment occupied sequence space, must send at least one reply

TCP reliability is built entirely on this contract: if you consume my sequence space, you owe me an ACK.


segment occupied sequence space → receiver's state changed
 → sender must know about it via an updated ACK
 → at minimum, send a bare ACK if nothing else to send


Generalizing: Every Sequence Space Byte Needs an ACK
This applies throughout the entire connection, not just the handshake:

```
DATA segment arrives (payload = "hello", 5 bytes):

  length_in_sequence_space() = 5   → must reply

  Why? Sender is holding "hello" in outstanding queue.
  It will retransmit every RTO until ACK arrives.
  If we process the data but never ACK it:
    → sender retransmits "hello" forever
    → receiver gets duplicates
    → connection degrades

```

```
FIN segment arrives:

  length_in_sequence_space() = 1   → must reply

  Why? Sender is waiting for its FIN to be ACKed.
  Without the ACK, sender retransmits FIN indefinitely.
  Connection teardown never completes.
```


[4] Drain sender queue → enrich each segment → push to _segments_out




##### Exact Trace of the Third ACK
```
segment_received(SYN+ACK):

[1] _receiver.segment_received(SYN+ACK)
      _syn_received = true
      _isn = ISN_s
      _ackno = ISN_s + 1          ← receiver now has valid ackno

[2] seg.header().ack == true
    _sender.ack_received(ISN_c+1, win)
      SYN segment removed from outstanding queue
      fill_window() → nothing to send
      _sender.segments_out() still empty

[3] SYN+ACK length_in_sequence_space() == 1  (SYN bit counts)  → must reply
    _sender.segments_out().empty() == true
    → _sender.send_empty_segment()
         seg.header().seqno = wrap(_next_seqno, _isn)
                            = ISN_c + 1
         no SYN, no FIN, no payload
         pushed to _sender._segments_out → bare seg queued (the seqno part) 

[4] push_segments_out(): enriches bare seg with ackno=ISN_s+1  (the ackno part)
    take seg from _sender._segments_out
    _receiver.ackno().has_value() == true
      seg.header().ack   = true
      seg.header().ackno = ISN_s + 1    ← server's SYN acknowledged
    seg.header().win = receiver window size
    push to _segments_out

Result on wire:
  seqno  = ISN_c + 1
  ackno  = ISN_s + 1
  ack    = true
  SYN    = false
  FIN    = false
  payload = empty

This is exactly the bare ACK that completes the three-way handshake.
```
### State Progression


```
CLIENT side:
  State               Trigger
  ─────────────────────────────────────────────────────
  CLOSED              (initial)
  SYN_SENT            connect() called → SYN sent
  ESTABLISHED         segment_received(SYN+ACK) → ACK sent

SERVER side:
  State               Trigger
  ─────────────────────────────────────────────────────
  LISTEN              (initial, waiting)
  SYN_RECEIVED        segment_received(SYN) → SYN+ACK sent
  ESTABLISHED         segment_received(ACK)
```



## TCP's Three Events and the Event Loop

### The Three Events in TCP

```
Event Type          Method Called           Triggered By
─────────────────────────────────────────────────────────
Segment arrives     segment_received()      Network / OS
Time passes         tick()                  Runtime timer
App writes data     write() / connect()     Application
```

These are the **only three ways** the outside world interacts with TCPConnection.

---

Who Sends These Events and When?

#### Event 1: `segment_received()` — Network delivers a segment

```
Network
  │
  ▼
OS receives IP/UDP packet
  │
  ▼
Runtime unpacks TCP segment from packet payload
  │
  ▼
TCPConnection::segment_received(seg)   ← your code gets called here
```

**Who triggers it:** The runtime/OS, whenever a packet physically arrives on the network interface.

**When:** Completely unpredictable. Could be 1ms from now, could be 10 seconds.

---

#### Event 2: `tick()` — Time passes

```
OS wall clock advances
  │
  ▼
Runtime measures elapsed milliseconds
  │
  ▼
TCPConnection::tick(ms_since_last_tick)   ← your code gets called here
```

**Who triggers it:** The runtime, periodically and unconditionally.

**When:** Roughly every few milliseconds, regardless of network activity.

**Purpose:** Retransmission timers, linger timeout, RTO backoff — none of these can rely on segments arriving to measure time.

---

#### Event 3: `write()` / `connect()` — Application produces data

```
Application calls write("hello")
  │
  ▼
TCPConnection::write(data)   ← your code gets called here
  │
  ▼
bytes enter outbound ByteStream
  │
  ▼
sender packetizes → segments pushed to _segments_out
```

**Who triggers it:** The application itself, directly.

**When:** Whenever the user/program has data to send.

---

### How Does the Client "Wait" for Events?

This is the deep question. The answer is: **the client does not wait inside TCPConnection at all**. The waiting happens in the layer *above* — in the event loop of the runtime that owns TCPConnection.

#### The Event Loop (in `tcp_sponge_socket.cc`)

In CS144, the runtime uses a loop roughly like this:

```cpp
// Simplified version of what the CS144 runtime does
while (tcp_connection.active()) {

    // Wait for ANY of these to become ready (using poll/epoll)
    poll({
        network_socket_fd,    // wait for incoming segment
        timer_fd,             // wait for tick interval
        stdin_fd              // wait for application data
    });

    // Handle whichever fd became ready:

    if (network_socket_fd is readable) {
        TCPSegment seg = read_segment_from_network();
        tcp_connection.segment_received(seg);
    }

    if (timer_fd fired) {
        size_t ms = time_elapsed();
        tcp_connection.tick(ms);
    }

    if (stdin_fd is readable) {
        string data = read_from_application();
        tcp_connection.write(data); 
    }

    // After any event, drain outgoing segments to network
    while (!tcp_connection.segments_out().empty()) {
        send_to_network(tcp_connection.segments_out().front());
        tcp_connection.segments_out().pop();
    }
}
```

The key system call here is `poll()` (or `epoll` / `select`). It **blocks the thread** until at least one file descriptor is ready, then wakes up and dispatches exactly one event. Then it goes back to sleep.

---

#### The Full Picture: Client Timeline

```
Time    Event Loop          TCPConnection           Network
────────────────────────────────────────────────────────────
t=0     app calls connect()
          └─► connect()
                fill_window() → SYN
                segments_out ← [SYN]
          drain segments_out
                └─────────────── SYN ──────────────────────►

t=0     poll() called
          blocks, waiting...

t=5ms   network_fd readable   ← SYN+ACK arrives ──────────
          segment_received(SYN+ACK)
                receiver gets SYN
                sender gets ACK
                → reply needed → ACK
                segments_out ← [ACK]
          drain segments_out
                └─────────────── ACK ──────────────────────►

t=5ms   poll() called again
          blocks, waiting...

t=6ms   timer_fd fires
          tick(1)
                no retransmit needed
                nothing to do

t=6ms   poll() called again
          blocks, waiting...

t=100ms stdin_fd readable
          app typed "hello"
          write("hello")
                bytes → outbound stream
                sender packetizes → DATA segment
                segments_out ← [DATA]
          drain segments_out
                └─────────────── DATA seq=1 len=5 ─────────►
```

---

#### Why `poll()` Instead of a `while(true)` Busy Loop?

A naive approach would be:

```cpp
// BAD: busy loop
while (true) {
    if (network has data) segment_received(...);
    if (timer fired)      tick(...);
    if (app has data)     write(...);
}
```

This wastes 100% CPU spinning even when nothing is happening.

`poll()` instead tells the kernel: **"wake me up only when something actually needs attention"**:

```
poll() system call
  │
  ▼
kernel puts thread to sleep
  │
  (thread uses 0% CPU here)
  │
  ▼
kernel wakes thread when:
  - a packet arrives on network_fd   → segment_received()
  - timer interval elapses           → tick()
  - application writes to stdin_fd   → write()
```

```
CPU usage:
  busy loop:   ████████████████████████████  100%
  poll-based:  █░░░░░░░░█░░░░░░░░█░░░░░░░░   ~5%
               ↑         ↑         ↑
             event     event     event
```

---

#### The Boundary: What Lives Where

```
┌──────────────────────────────────────────────┐
│              Runtime / Event Loop             │
│  (tcp_sponge_socket.cc, uses poll())          │
│                                               │
│  Knows about: file descriptors, OS timers     │
│  Responsible for: WAITING, DISPATCHING        │
│                                               │
│  ┌────────────────────────────────────────┐   │
│  │           TCPConnection                │   │
│  │                                        │   │
│  │  Knows about: TCP protocol state       │   │
│  │  Responsible for: REACTING to events  │   │
│  │                                        │   │
│  │  Does NOT know about: file descriptors │   │
│  │  Does NOT know about: OS timers        │   │
│  │  Does NOT block or wait               │   │
│  └────────────────────────────────────────┘   │
└──────────────────────────────────────────────┘
```

TCPConnection is **pure protocol logic**. It never calls `read()`, `sleep()`, or `poll()`. It only reacts to events that the runtime delivers to it. This separation is what makes TCPConnection independently testable — the test suite can inject segments and tick calls without any real network or OS timer.

---

#### Summary

```
Who waits?        The runtime event loop (poll())
Who dispatches?   The runtime, based on which fd became ready
Who reacts?       TCPConnection, via segment_received() / tick() / write()
Does connect() wait for SYN-ACK?   NO. It fires SYN and returns.
Does segment_received() wait?      NO. It processes and returns.
```

The entire TCP state machine advances **one event at a time**, each event handled in microseconds, with the thread sleeping between events via `poll()`.

Would you like to look at the actual `tcp_sponge_socket.cc` event loop code, or go deeper into how `poll()` works at the Linux kernel level?



# TCP Connection Teardown:

There are two ways a connection can end. 






## 1. UNCLEAN SHUTDOWN: RST (error state) 

In an unclean shutdown, the TCPConnection either sends or receives a segment with the `RST` flag set. In this case, the outbound and inbound ByteStreams should both be in the error state, and active() can return false immediately.


### What Is the Error State?

The error state is a flag on a `ByteStream` that signals:

```
"Something went catastrophically wrong.
 This stream is permanently broken.
 No more reading or writing is meaningful."
```

In CS144, `ByteStream` has a method:

```cpp
void ByteStream::set_error() {
    _error = true;
}

bool ByteStream::error() const {
    return _error;
}
```

Once set, it never clears. The stream is dead.

---

### What Triggers the Error State?

Only one thing in TCP sets the error state: **receiving or sending an RST segment**.

```
RST = Reset
    = "Something is badly wrong, abort immediately"
    = the TCP equivalent of slamming down the phone
```

Two situations cause RST:

```
Situation 1: We RECEIVE a RST from remote peer
  → remote peer decided to abort the connection
  → could be: invalid segment, connection refused,
              too many retransmissions on their side,
              application crashed

Situation 2: We SEND a RST to remote peer
  → we decided to abort
  → could be: too many retransmissions on our side
              (MAX_RETX_ATTEMPTS exceeded)
  → destructor called while connection still active
```

---

### What Happens When RST Is Received

```
segment_received(RST):

  RST flag set → unclean shutdown

  inbound ByteStream:  set_error()   ← application reads get error
  outbound ByteStream: set_error()   ← application writes get error
  _rst_flag = true                   ← connection marked dead

  active() returns false immediately
  No lingering. No waiting. Dead now.
```

```
Us                              Remote
│                               │
│  data transfer happening...   │
│                               │
│◄──── RST ─────────────────── │  ← remote aborts
│                               │
│  inbound stream  → ERROR      │
│  outbound stream → ERROR      │
│  active() → false             │
│  connection dead              │
```

---

### Error State vs Clean Shutdown: The Difference

```
                Clean Shutdown          Error State (RST)
────────────────────────────────────────────────────────────────
Cause           FIN exchange            RST segment
Data delivery   all data delivered      data may be lost
Stream state    input_ended() = true    error() = true
active()        false after linger      false immediately
Application     reads remaining data    reads return error
knows?          graceful EOF            abrupt termination
```

Clean shutdown is like finishing a phone call properly:

```
"Goodbye" → "Goodbye" → both hang up
All words were heard.
```

Error state is like the line going dead mid-sentence:

```
"Hey I wanted to tell you ab—" → DEAD LINE
Words may have been lost.
```

---

### How Your Code Handles RST

In `segment_received()` you need to add RST handling at the very top:

```cpp
void TCPConnection::segment_received(const TCPSegment &seg) {

    // RST: immediate unclean shutdown
    if (seg.header().rst) {
        _sender.stream_in().set_error();       // outbound stream → error
        _receiver.stream_out().set_error();    // inbound stream → error
        _rst = true;                           // mark connection dead
        return;                                // do nothing else
    }

    // ... rest of segment_received ...
}
```

And when sending RST (too many retransmissions):

```cpp
void TCPConnection::send_rst() {
    _sender.send_empty_segment();          // get a segment with proper seqno
    TCPSegment seg = _sender.segments_out().front();
    _sender.segments_out().pop();

    seg.header().rst = true;               // set RST flag

    // also set ACK if receiver has ackno
    if (_receiver.ackno().has_value()) {
        seg.header().ack   = true;
        seg.header().ackno = _receiver.ackno().value();
    }

    _segments_out.push(seg);               // send it

    // kill both streams
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _rst = true;
}
```

---

### Where RST Is Triggered in `tick()`

```cpp
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _sender.tick(ms_since_last_tick);
    _time_since_last_segment_received += ms_since_last_tick;

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        // too many retransmits → remote peer is probably dead
        // abort with RST
        send_rst();
        return;
    }

    // check clean shutdown condition...
}
```

```
Retransmission timeline:

t=0     send DATA
t=1000  no ACK → retransmit (RTO=1000)
t=3000  no ACK → retransmit (RTO=2000, backoff)
t=7000  no ACK → retransmit (RTO=4000, backoff)
t=15000 no ACK → retransmit (RTO=8000, backoff)
...
t=N     consecutive_retransmissions > 8
        → send RST
        → set error state
        → connection dead
```

---

### Complete Picture

```
TCP Connection Ending:

Two paths:
                    ┌─────────────────────────────────┐
                    │         Connection Ends          │
                    └─────────────────────────────────┘
                              │           │
              ┌───────────────┘           └───────────────┐
              ▼                                           ▼
    CLEAN SHUTDOWN                             UNCLEAN SHUTDOWN
    (FIN exchange)                             (RST)
              │                                           │
    All data delivered                         Data may be lost
    Streams end gracefully                     Streams → error state
    active() = false after                     active() = false
    linger or passive close                    immediately
              │                                           │
    Two Generals Problem:                      Causes:
    cannot 100% guarantee                      - RST received from peer
    both sides agree                           - Too many retransmits
    Solution: linger 10×RTO                    - Destructor while active
    as close as possible
```

The Two Generals Problem explains **why** clean shutdown needs the linger timeout. The error state explains **what happens** when things go wrong abruptly. They are the two failure modes TCP must handle — one fundamental and mathematical, one practical and operational.

## 2. CLEAN SHUTDOWN   


  A clean shutdown is how we get to "done" (active() = false) without an error. This is more complicated, but it's a beautiful thing because it ensures as much as possible that each of the two ByteStreams has been reliably delivered completely to the receiving peer. In the next section (§§5.1), we give the practical upshot for when a clean shutdown happens, so feel free to skip ahead if you like.


### Why Is Closing Complicated?

Closing a TCP connection seems simple:

```
A: "I'm done"
B: "OK I'm done too"
→ both close
```

But TCP runs over an **unreliable network**. The last message might get lost:

```
A: "I'm done, goodbye"  ──────────────────► B
A: "OK confirmed"       ◄──────────────────  B
                        ←----LOST IN NETWORK
A never gets this.
A doesn't know if B received the goodbye.
```

This is the **Two Generals Problem** — it is mathematically impossible to guarantee both sides agree they are done over an unreliable channel. TCP gets as close as practically possible.

---

#### The Two Streams: Independent Directions

First, understand that TCP has **two independent byte streams**:

```
┌─────────────────────────────────────────────────────┐
│                  TCP Connection                      │
│                                                      │
│  A ──────── outbound stream ────────────────► B      │
│                                                      │
│  A ◄─────── inbound stream  ──────────────── B      │
│                                                      │
└─────────────────────────────────────────────────────┘
```

Each stream ends independently. One side can stop sending while still receiving. This is called **half-close**.

```
A calls end_input_stream()     →  A's outbound stream ends
                                  B's inbound stream ends
                                  A can still RECEIVE from B
```

So "closing the connection" really means: **both streams must end in both directions**.

---

#### The Original The Two Generals Story

```
Two armies want to attack a city simultaneously.
If both attack at the same time → they win.
If only one attacks → they lose.

Army A                    City                    Army B
  │                        │                        │
  │    messenger ─────────►│──────────────────────► │
  │                        │                        │
  │◄──────────────────────│◄───────── reply ────── │
  │                        │                        │

Problem: the messenger might be captured (message lost).
```

Army A sends "attack at dawn" to Army B. But:

```
Army A thinks:
  "Did Army B get my message?
   I don't know. Better send a confirmation request."

Army B receives message, sends back "confirmed".
  "Did Army A get my confirmation?
   I don't know. Better send another confirmation."

Army A receives confirmation, sends "got your confirmation".
  "Did Army B get THIS message?
   I don't know..."
```

This goes **forever**. There is no final message that can guarantee both sides are synchronized, because every final message might be lost.

```
This is mathematically proven to be unsolvable
over an unreliable communication channel.
```

---

#### How This Maps to TCP

Replace the armies with TCP peers:

```
Two Generals Problem          TCP Equivalent
─────────────────────────────────────────────────────
Two armies                    Two TCP endpoints
Attack simultaneously         Close connection cleanly
Messenger captured            Packet lost in network
"Attack at dawn"              FIN segment
"Confirmed"                   ACK of FIN
City walls                    The unreliable Internet
```

The exact same problem appears at TCP teardown:

```
Us                              Remote
│                               │
│──── FIN ──────────────────────►│
│◄──── ACK(FIN) ─────────────── │
│◄──── FIN ─────────────────────│
│──── ACK(FIN) ─────────────────►│
                 ↑
         this might be lost

Remote thinks:
  "Did they get my FIN ACK?
   I don't know. I'll retransmit FIN."

Us think:
  "Did they get our final ACK?
   We don't know either."
```

Neither side can be **100% certain** the other side is done. Just like the generals.

---

#### Why TCP Cannot Fully Solve It

```
To be 100% certain, we would need:

Us: sends ACK of remote's FIN
Remote: sends ACK of our ACK
Us: sends ACK of remote's ACK of our ACK
Remote: sends ACK of our ACK of remote's ACK of our ACK
...
→ infinite chain of acknowledgments
→ impossible
```

#### TCP's Practical Solution: Stop Trying to Be Perfect

Since perfect is impossible, TCP uses time as a substitute for certainty:

TCP's practical solution is the **linger timeout**:

```
Instead of:
  "I am 100% certain Remote got my ACK"  → impossible

TCP says:

  "I cannot be 100% certain.
  But if I wait 10×RTO, 
  If Remote had not gotten my ACK, 
  it would have retransmitted FIN by now.
  I heard nothing,
  Therefore the remote probably got my last ACK.
  Close enough. Move on."


Us                              Remote
│──── FIN ──────────────────────►│
│◄──── ACK ─────────────────────│
│◄──── FIN ─────────────────────│
│──── ACK ──────────────────────►│  ← might be lost
│                               │
│  waiting...                   │  Remote retransmits if lost
│  waiting...                   │  We ACK again if retransmit arrives
│  waiting...                   │
│  10 × RTO elapsed             │
│  silence from Remote          │
│  → Remote got our ACK         │
│  → close ✓                    │

```

This is the engineering tradeoff: accept a tiny probability of imperfect shutdown rather than waiting forever.

The silence is the signal. If Remote did not get the ACK, it would be retransmitting FIN. Silence means it probably got it.

---


### The Four Prerequisites

Think of it as a checklist that must be fully ticked before declaring done:

```
From LOCAL peer's perspective:

Prereq 1: inbound stream fully received
          ─────────────────────────────
          Remote peer sent FIN.
          We reassembled everything including FIN.
          _receiver: stream_out().input_ended() == true

Prereq 2: outbound stream fully sent
          ────────────────────────────
          Local app called end_input_stream().
          We sent FIN segment to remote peer.
          _sender: _fin_sent == true

Prereq 3: outbound stream fully acknowledged
          ────────────────────────────────────
          Remote peer ACKed our FIN and all data.
          _sender: bytes_in_flight() == 0

Prereq 4: remote peer KNOWS we received everything
          ─────────────────────────────────────────
          This is the hard one. (explained below)
```

Prerequisites 1-3 are straightforward to check. Prereq 4 is the brain-bender.

---

### Why Prereq 4 Is Hard

After Prereqs 1-3 are satisfied, consider what has happened:

```
Timeline:
  ...data transfer...
  Remote sends FIN  ──────────────────────────────► Us
  Us sends ACK(FIN) ──────────────────────────────► Remote
  Us sends our FIN  ──────────────────────────────► Remote
  Remote sends ACK  ──────────────────────────────► Us
                                    ↑
                             did this arrive?
                             WE DON'T KNOW
```

We ACKed the remote's FIN. But does the remote **know** we ACKed it? TCP doesn't ACK ACKs. So we cannot be 100% certain.

If the remote did NOT receive our ACK of its FIN:

```
Remote's view:               Our view:
  "I sent FIN"                 "prereqs 1-3 done, closing"
  "No ACK received"
  "Must retransmit FIN"
  FIN ──────────────────────► Us (but we already closed!)
  
  We are gone. Nobody ACKs the retransmitted FIN.
  Remote gets stuck retransmitting forever.
```

This is the problem Prereq 4 solves.

---

### Two Solutions to Prereq 4

```

Active close:   We closed first
  our last ACK carries NO new seqno
  Our final ACK might be lost. Remote cannot prove it arrived
  → must linger
  Solution: linger 10×RTO.                     
   Use silence as proxy for certainty.          
  _linger_after_streams_finish = true 

Passive close: Remote closed first 
  our FIN carries new seqno + ACK of Remote's FIN
  Remote ACKs our FIN → proves Remote saw our ACK
  Can close immediately.
  → no linger needed
  _linger_after_streams_finish = false 
```

#### Option B: Passive Close (Simple Case)

**The remote peer was the FIRST to end its stream.**

```
Timeline:
  Remote sends FIN first  ─────────────────────────► Us    [Prereq 1 ✓]
  We send ACK(FIN)        ─────────────────────────► Remote
  We send our own FIN     ─────────────────────────► Remote [Prereq 2 ✓]
  Remote sends ACK        ─────────────────────────► Us    [Prereq 3 ✓]
```

Now here is the key insight:

```
Our FIN segment carried: seqno = (something new)
                         ackno = remote_FIN_seqno + 1

Remote ACKed our FIN.

To ACK our FIN, remote had to receive it.
Our FIN contained ackno that acknowledged remote's FIN.
Therefore: remote SAW that we acknowledged its FIN.
Therefore: remote KNOWS we got its FIN.
Therefore: remote will NOT retransmit its FIN.
Therefore: we can close immediately. 
No lingering needed.
```

```
_linger_after_streams_finish = false
→ when prereqs 1-3 satisfied → close immediately
```

When does this apply?

```
inbound stream ended (Prereq 1 satisfied)
BEFORE
we sent our FIN (Prereq 2 satisfied)

→ remote sent FIN first → passive close → no linger needed
```

In code:

```cpp
// In segment_received(), when we detect inbound stream just ended:
if (_receiver.stream_out().input_ended()) {
    if (!_sender.fin_sent()) {
        // remote ended first, before we sent our FIN
        _linger_after_streams_finish = false;
    }
}
```

#### Option A: Active Close (Hard Case, Requires Lingering)

**We sent our FIN BEFORE receiving the remote's FIN.**

```
Timeline:
  We send our FIN    ─────────────────────────────► Remote  [Prereq 2 ✓]
  Remote sends ACK   ─────────────────────────────► Us      [Prereq 3 ✓]
  Remote sends FIN   ─────────────────────────────► Us      [Prereq 1 ✓]
  We send ACK(FIN)   ─────────────────────────────► Remote
                                     ↑
                              this might be lost
```

Now prereqs 1-3 are all satisfied. But:

```
Remote's view:
  "I sent FIN"
  "Waiting for ACK..."
  
  Did our ACK(FIN) arrive? We don't know.
  
  If it was lost:
    Remote retransmits FIN
    → we need to still be alive to ACK it
    → if we closed immediately, remote gets stuck
```

Solution: **linger**. Stay alive for `10 × initial_RTO` milliseconds after Prereqs 1-3 are satisfied:

```
10 × RTO = roughly 10 × 1000ms = 10 seconds (TIME_WAIT in real TCP)

During this time:
  - connection still "active"
  - still listening for retransmitted FINs
  - still sending ACKs if needed
  - after timeout: confident remote got our ACK → close
```

Why 10× RTO? It is enough time for:

```
Our ACK travels to remote       ~1 RTT
If lost, remote retransmits FIN ~1 RTO
Retransmitted FIN reaches us    ~1 RTT
We ACK it, remote gets ACK      ~1 RTT

Several RTO cycles fit in 10× RTO.
If remote was going to retransmit, it would have by now.
Silence = remote got our ACK.
```

---

### The Complete Decision Tree

```
Are Prereqs 1-3 all satisfied?
        │
        NO → stay active, keep running
        │
        YES
        │
        ▼
Is _linger_after_streams_finish == false?
        │
        YES → close immediately (active() = false)
        │     (passive close, Option B)
        NO
        │
        ▼
Has 10 × _cfg.rt_timeout elapsed since last segment received?
        │
        NO → stay active, keep lingering
        │
        YES → close (active() = false)
              (linger complete, Option A)
```

---

### Concrete Timeline: Both Scenarios

#### Scenario 1: Passive Close (no linger)

```
Us                              Remote
│                               │
│◄──── data ─────────────────── │
│──── ACK ──────────────────────►│
│◄──── FIN ─────────────────────│  ← remote ends first
│──── ACK(FIN) ─────────────────►│  ← Prereq 1 ✓
│                               │
│  app calls end_input_stream() │
│──── FIN ──────────────────────►│  ← Prereq 2 ✓
│  _linger = false (set earlier) │
│◄──── ACK(FIN) ─────────────── │  ← Prereq 3 ✓
│                               │
│  Prereqs 1-3 ✓                │
│  _linger = false              │
│  → close immediately ✓        │
```

#### Scenario 2: Active Close (must linger)

```
Us                              Remote
│                               │
│  app calls end_input_stream() │
│──── FIN ──────────────────────►│  ← Prereq 2 ✓, we sent FIN first
│◄──── ACK(FIN) ─────────────── │  ← Prereq 3 ✓
│◄──── FIN ─────────────────────│  ← Prereq 1 ✓
│──── ACK(FIN) ─────────────────►│  ← might be lost
│                               │
│  Prereqs 1-3 ✓                │
│  _linger = true               │
│  start 10×RTO timer...        │
│                               │
│  (silence from remote)        │  ← remote got our ACK
│  (10×RTO elapses)             │
│  → close ✓                    │
```

---

### How `active()` Implements This

```cpp
bool TCPConnection::active() const {

    // unclean shutdown: RST sent or received
    if (_rst) return false;

    // Prereq 1: inbound stream fully assembled and ended
    bool prereq1 = _receiver.stream_out().input_ended();

    // Prereq 2: outbound FIN sent
    bool prereq2 = _sender.fin_sent();

    // Prereq 3: all outbound data acknowledged
    bool prereq3 = (_sender.bytes_in_flight() == 0);

    if (!prereq1 || !prereq2 || !prereq3) {
        return true;   // not done yet
    }

    // Prereqs 1-3 all satisfied:
    if (!_linger_after_streams_finish) {
        return false;  // passive close → done immediately
    }

    // active close → must linger
    return _time_since_last_segment_received < 10 * _cfg.rt_timeout;
}
```

---

### Summary

```
Why complicated?
  TCP is reliable over unreliable network.
  "Done" requires BOTH sides to agree.
  Last message might be lost.
  Must handle retransmissions even during shutdown.

Two cases:
  Passive close  → remote sent FIN first
                 → we know remote got our ACK (via our FIN+ACK exchange)
                 → close immediately, no linger
                 → _linger_after_streams_finish = false

  Active close   → we sent FIN first
                 → our final ACK might be lost
                 → must stay alive 10×RTO to handle retransmits
                 → _linger_after_streams_finish = true (default)

The _linger flag:
  starts true
  set to false ONLY when: inbound ended BEFORE we sent our FIN
```

The elegance of TCP teardown is that it handles an impossible problem (Two Generals) as well as practically possible, using time as a proxy for certainty.



# Test

## Start Test

```bash
chmod +x /home/cs144/computer_network/sponge/tun.sh
chmod +x /home/cs144/computer_network/sponge/tun.sh

cs144@cs144vm:~/computer_network/sponge/build$ make check_lab4
[100%] Testing the TCP connection...
[/home/cs144/computer_network/sponge/tun.sh] Bringing up tunnels 144 145:
[sudo] password for cs144: 
Test project /home/cs144/computer_network/sponge/build
        Start   1: t_wrapping_ints_cmp
  1/162 Test   #1: t_wrapping_ints_cmp ..............   Passed    0.00 sec
        Start   2: t_wrapping_ints_unwrap
  2/162 Test   #2: t_wrapping_ints_unwrap ...........   Passed    0.02 sec
        Start   3: t_wrapping_ints_wrap
  3/162 Test   #3: t_wrapping_ints_wrap .............   Passed    0.01 sec
        Start   4: t_wrapping_ints_roundtrip
  4/162 Test   #4: t_wrapping_ints_roundtrip ........   Passed    0.14 sec
        Start   5: t_recv_connect
  5/162 Test   #5: t_recv_connect ...................   Passed    0.01 sec
        Start   6: t_recv_transmit
  6/162 Test   #6: t_recv_transmit ..................   Passed    0.06 sec
        Start   7: t_recv_window
  7/162 Test   #7: t_recv_window ....................   Passed    0.01 sec
        Start   8: t_recv_reorder
  8/162 Test   #8: t_recv_reorder ...................   Passed    0.01 sec
        Start   9: t_recv_close
  9/162 Test   #9: t_recv_close .....................   Passed    0.01 sec
        Start  10: t_recv_special
 10/162 Test  #10: t_recv_special ...................   Passed    0.01 sec
        Start  11: t_send_connect
 11/162 Test  #11: t_send_connect ...................   Passed    0.01 sec
        Start  12: t_send_transmit
 12/162 Test  #12: t_send_transmit ..................   Passed    0.04 sec
        Start  13: t_send_retx
 13/162 Test  #13: t_send_retx ......................   Passed    0.01 sec
        Start  14: t_send_window
 14/162 Test  #14: t_send_window ....................   Passed    0.10 sec
        Start  15: t_send_ack
 15/162 Test  #15: t_send_ack .......................   Passed    0.01 sec
        Start  16: t_send_close
 16/162 Test  #16: t_send_close .....................   Passed    0.01 sec
        Start  17: t_send_extra
 17/162 Test  #17: t_send_extra .....................   Passed    0.01 sec
        Start  18: t_strm_reassem_single
 18/162 Test  #18: t_strm_reassem_single ............   Passed    0.01 sec
        Start  19: t_strm_reassem_seq
 19/162 Test  #19: t_strm_reassem_seq ...............   Passed    0.01 sec
        Start  20: t_strm_reassem_dup
 20/162 Test  #20: t_strm_reassem_dup ...............   Passed    0.01 sec
        Start  21: t_strm_reassem_holes
 21/162 Test  #21: t_strm_reassem_holes .............   Passed    0.01 sec
        Start  22: t_strm_reassem_many
 22/162 Test  #22: t_strm_reassem_many ..............   Passed    0.99 sec
        Start  23: t_strm_reassem_overlapping
 23/162 Test  #23: t_strm_reassem_overlapping .......   Passed    0.01 sec
        Start  24: t_strm_reassem_win
 24/162 Test  #24: t_strm_reassem_win ...............   Passed    1.10 sec
        Start  25: t_strm_reassem_cap
 25/162 Test  #25: t_strm_reassem_cap ...............   Passed    0.08 sec
        Start  26: t_byte_stream_construction
 26/162 Test  #26: t_byte_stream_construction .......   Passed    0.01 sec
        Start  27: t_byte_stream_one_write
 27/162 Test  #27: t_byte_stream_one_write ..........   Passed    0.01 sec
        Start  28: t_byte_stream_two_writes
 28/162 Test  #28: t_byte_stream_two_writes .........   Passed    0.01 sec
        Start  29: t_byte_stream_capacity
 29/162 Test  #29: t_byte_stream_capacity ...........   Passed    0.52 sec
        Start  30: t_byte_stream_many_writes
 30/162 Test  #30: t_byte_stream_many_writes ........   Passed    0.01 sec
        Start  31: t_webget
 31/162 Test  #31: t_webget .........................   Passed    1.61 sec
        Start  34: t_tcp_parser
 32/162 Test  #34: t_tcp_parser .....................   Passed    0.04 sec
        Start  35: t_ipv4_parser
 33/162 Test  #35: t_ipv4_parser ....................   Passed    0.01 sec
        Start  36: t_active_close
 34/162 Test  #36: t_active_close ...................***Failed    0.01 sec
Test Failure on expectation:
        Expectation: exactly one segment sent with (A=1,R=0,S=0,F=1,ackno=1,seqno=1,)

Failure message:
        The TCP should have produced a segment that existed, but it did not

List of steps that executed successfully:
        Action:      connect
        Expectation: exactly one segment sent with (A=0,R=0,S=1,F=0,seqno=0,payload_size=0,)
        Action:      packet arrives: Header(flags=SA,seqno=0,ack=1,win=137) with no payload
        Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=1,payload_size=0,)
        Action:      close

The TCP should have produced a segment that existed, but it did not

        Start  37: t_passive_close
 35/162 Test  #37: t_passive_close ..................***Failed    0.01 sec
Test Failure on expectation:
        Expectation: exactly one segment sent with (A=1,R=0,S=0,F=1,ackno=2,seqno=1,)

Failure message:
        The TCP should have produced a segment that existed, but it did not

List of steps that executed successfully:
        Action:      connect
        Expectation: exactly one segment sent with (A=0,R=0,S=1,F=0,seqno=0,payload_size=0,)
        Action:      packet arrives: Header(flags=SA,seqno=0,ack=1,win=137) with no payload
        Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=1,payload_size=0,)
        Action:      packet arrives: Header(flags=AF,seqno=1,ack=1,win=137) with no payload
        Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=2,)
        Action:      close

The TCP should have produced a segment that existed, but it did not

        Start  38: t_ack_rst
 36/162 Test  #38: t_ack_rst ........................***Failed    0.01 sec
Test 1
Test 2
Test 3
Test Failure on expectation:
        Expectation: TCP in state sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1

Failure message:
        The TCP was in state `sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1`

List of steps that executed successfully:
        Action:      listen
        Action:      packet arrives: Header(flags=A,seqno=2147483648,ack=2147483648,win=137) with no payload

The TCP was in state `sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1` (ack_listen_test called from line 131)

        Start  39: t_ack_rst_win
 37/162 Test  #39: t_ack_rst_win ....................   Passed    0.01 sec
        Start  40: t_connect
 38/162 Test  #40: t_connect ........................***Failed    0.01 sec
Test Failure on expectation:
        Expectation: TCP in state sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1

Failure message:
        The TCP was in state `sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1`

List of steps that executed successfully:
        Action:      connect
        Action:      1ms pass
        Expectation: exactly one segment sent with (A=0,S=1,)

The TCP was in state `sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1`

        Start  41: t_listen
 39/162 Test  #41: t_listen .........................***Failed    0.01 sec
Test Failure on expectation:
        Expectation: TCP in state sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1

Failure message:
        The TCP was in state `sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1`

List of steps that executed successfully:
        Action:      listen

The TCP was in state `sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1`

        Start  42: t_winsize
 40/162 Test  #42: t_winsize ........................***Failed    0.01 sec
Test Failure on expectation:
        Expectation: TCP in state sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=1, linger_after_streams_finish=1

Failure message:
        The TCP was in state `sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=1, linger_after_streams_finish=1`

List of steps that executed successfully:
        Action:      listen
        Action:      packet arrives: Header(flags=S,seqno=1617239019,ack=0,win=137) with no payload
        Expectation: exactly one segment sent with (A=1,ackno=1617239020,win=3418,)
        Action:      packet arrives: Header(flags=A,seqno=1617239020,ack=3670470556,win=27372) with no payload
        Expectation: no (more) segments sent

The TCP was in state `sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=1, linger_after_streams_finish=1`

        Start  43: t_retx
 41/162 Test  #43: t_retx ...........................***Failed    0.01 sec
  check_segment
  check_segment
  check_segment
  check_segment
  check_segment
  check_segment
  check_segment
  check_segment
  check_segment
Test Failure on expectation:
        Expectation: TCP in state sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=1, linger_after_streams_finish=1

Failure message:
        The TCP was in state `sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=1, linger_after_streams_finish=1`

List of steps that executed successfully:
        Action:      connect
        Expectation: exactly one segment sent with (A=0,R=0,S=1,F=0,seqno=1450750972,payload_size=0,)
        Action:      packet arrives: Header(flags=SA,seqno=1450750972,ack=1450750973,win=137) with no payload
        Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=1450750973,payload_size=0,)
        Action:      write (4 bytes) [61, 73, 64, 66]
        Action:      1ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent
        Action:      998ms pass
        Expectation: no (more) segments sent
        Action:      2ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent
        Action:      10100ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent
        Action:      3998ms pass
        Expectation: no (more) segments sent
        Action:      2ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent
        Action:      7997ms pass
        Expectation: no (more) segments sent
        Action:      3ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent
        Action:      15996ms pass
        Expectation: no (more) segments sent
        Action:      4ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent
        Action:      31995ms pass
        Expectation: no (more) segments sent
        Action:      5ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent
        Action:      63994ms pass
        Expectation: no (more) segments sent
        Action:      6ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent
        Action:      127993ms pass
        Expectation: no (more) segments sent
        Action:      7ms pass
        Expectation: segment sent with (A=1,payload_size=4,data="asdf",)
        Expectation: no (more) segments sent

The TCP was in state `sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`stream ongoing`, receiver=`SYN received (ackno exists), and input to stream hasn't ended`, active=1, linger_after_streams_finish=1`

        Start  44: t_retx_win
 42/162 Test  #44: t_retx_win .......................   Passed    0.01 sec
        Start  45: t_loopback
 43/162 Test  #45: t_loopback .......................   Passed    0.83 sec
        Start  46: t_loopback_win
 44/162 Test  #46: t_loopback_win ...................   Passed    0.58 sec
        Start  47: t_reorder
 45/162 Test  #47: t_reorder ........................   Passed    1.12 sec
        Start  48: t_address_dt
 46/162 Test  #48: t_address_dt .....................   Passed    0.05 sec
        Start  49: t_parser_dt
 47/162 Test  #49: t_parser_dt ......................   Passed    0.01 sec
        Start  50: t_socket_dt
 48/162 Test  #50: t_socket_dt ......................   Passed    0.01 sec
        Start  51: t_udp_client_send
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 49/162 Test  #51: t_udp_client_send ................***Not Run   0.00 sec
        Start  52: t_udp_server_send
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 50/162 Test  #52: t_udp_server_send ................***Not Run   0.00 sec
        Start  53: t_udp_client_recv
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 51/162 Test  #53: t_udp_client_recv ................***Not Run   0.00 sec
        Start  54: t_udp_server_recv
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 52/162 Test  #54: t_udp_server_recv ................***Not Run   0.00 sec
        Start  55: t_udp_client_dupl
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 53/162 Test  #55: t_udp_client_dupl ................***Not Run   0.00 sec
        Start  56: t_udp_server_dupl
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 54/162 Test  #56: t_udp_server_dupl ................***Not Run   0.00 sec
        Start  57: t_ucS_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 55/162 Test  #57: t_ucS_1M_32k .....................***Not Run   0.00 sec
        Start  58: t_ucS_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 56/162 Test  #58: t_ucS_128K_8K ....................***Not Run   0.00 sec
        Start  59: t_ucS_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 57/162 Test  #59: t_ucS_16_1 .......................***Not Run   0.00 sec
        Start  60: t_ucS_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 58/162 Test  #60: t_ucS_32K_d ......................***Not Run   0.00 sec
        Start  61: t_ucR_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 59/162 Test  #61: t_ucR_1M_32k .....................***Not Run   0.00 sec
        Start  62: t_ucR_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 60/162 Test  #62: t_ucR_128K_8K ....................***Not Run   0.00 sec
        Start  63: t_ucR_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 61/162 Test  #63: t_ucR_16_1 .......................***Not Run   0.00 sec
        Start  64: t_ucR_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 62/162 Test  #64: t_ucR_32K_d ......................***Not Run   0.00 sec
        Start  65: t_ucD_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 63/162 Test  #65: t_ucD_1M_32k .....................***Not Run   0.00 sec
        Start  66: t_ucD_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 64/162 Test  #66: t_ucD_128K_8K ....................***Not Run   0.00 sec
        Start  67: t_ucD_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 65/162 Test  #67: t_ucD_16_1 .......................***Not Run   0.00 sec
        Start  68: t_ucD_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 66/162 Test  #68: t_ucD_32K_d ......................***Not Run   0.00 sec
        Start  69: t_usS_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 67/162 Test  #69: t_usS_1M_32k .....................***Not Run   0.00 sec
        Start  70: t_usS_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 68/162 Test  #70: t_usS_128K_8K ....................***Not Run   0.00 sec
        Start  71: t_usS_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 69/162 Test  #71: t_usS_16_1 .......................***Not Run   0.00 sec
        Start  72: t_usS_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 70/162 Test  #72: t_usS_32K_d ......................***Not Run   0.00 sec
        Start  73: t_usR_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 71/162 Test  #73: t_usR_1M_32k .....................***Not Run   0.00 sec
        Start  74: t_usR_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 72/162 Test  #74: t_usR_128K_8K ....................***Not Run   0.00 sec
        Start  75: t_usR_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 73/162 Test  #75: t_usR_16_1 .......................***Not Run   0.00 sec
        Start  76: t_usR_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 74/162 Test  #76: t_usR_32K_d ......................***Not Run   0.00 sec
        Start  77: t_usD_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 75/162 Test  #77: t_usD_1M_32k .....................***Not Run   0.00 sec
        Start  78: t_usD_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 76/162 Test  #78: t_usD_128K_8K ....................***Not Run   0.00 sec
        Start  79: t_usD_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 77/162 Test  #79: t_usD_16_1 .......................***Not Run   0.00 sec
        Start  80: t_usD_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 78/162 Test  #80: t_usD_32K_d ......................***Not Run   0.00 sec
        Start  81: t_ucS_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 79/162 Test  #81: t_ucS_128K_8K_l ..................***Not Run   0.00 sec
        Start  82: t_ucS_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 80/162 Test  #82: t_ucS_128K_8K_L ..................***Not Run   0.00 sec
        Start  83: t_ucS_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 81/162 Test  #83: t_ucS_128K_8K_lL .................***Not Run   0.00 sec
        Start  84: t_ucR_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 82/162 Test  #84: t_ucR_128K_8K_l ..................***Not Run   0.00 sec
        Start  85: t_ucR_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 83/162 Test  #85: t_ucR_128K_8K_L ..................***Not Run   0.00 sec
        Start  86: t_ucR_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 84/162 Test  #86: t_ucR_128K_8K_lL .................***Not Run   0.00 sec
        Start  87: t_ucD_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 85/162 Test  #87: t_ucD_128K_8K_l ..................***Not Run   0.00 sec
        Start  88: t_ucD_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 86/162 Test  #88: t_ucD_128K_8K_L ..................***Not Run   0.00 sec
        Start  89: t_ucD_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 87/162 Test  #89: t_ucD_128K_8K_lL .................***Not Run   0.00 sec
        Start  90: t_usS_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 88/162 Test  #90: t_usS_128K_8K_l ..................***Not Run   0.00 sec
        Start  91: t_usS_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 89/162 Test  #91: t_usS_128K_8K_L ..................***Not Run   0.00 sec
        Start  92: t_usS_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 90/162 Test  #92: t_usS_128K_8K_lL .................***Not Run   0.00 sec
        Start  93: t_usR_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 91/162 Test  #93: t_usR_128K_8K_l ..................***Not Run   0.00 sec
        Start  94: t_usR_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 92/162 Test  #94: t_usR_128K_8K_L ..................***Not Run   0.00 sec
        Start  95: t_usR_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 93/162 Test  #95: t_usR_128K_8K_lL .................***Not Run   0.00 sec
        Start  96: t_usD_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 94/162 Test  #96: t_usD_128K_8K_l ..................***Not Run   0.00 sec
        Start  97: t_usD_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 95/162 Test  #97: t_usD_128K_8K_L ..................***Not Run   0.00 sec
        Start  98: t_usD_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 96/162 Test  #98: t_usD_128K_8K_lL .................***Not Run   0.00 sec
        Start  99: t_ipv4_client_send
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 97/162 Test  #99: t_ipv4_client_send ...............***Not Run   0.00 sec
        Start 100: t_ipv4_server_send
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 98/162 Test #100: t_ipv4_server_send ...............***Not Run   0.00 sec
        Start 101: t_ipv4_client_recv
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
 99/162 Test #101: t_ipv4_client_recv ...............***Not Run   0.00 sec
        Start 102: t_ipv4_server_recv
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
100/162 Test #102: t_ipv4_server_recv ...............***Not Run   0.00 sec
        Start 103: t_ipv4_client_dupl
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
101/162 Test #103: t_ipv4_client_dupl ...............***Not Run   0.00 sec
        Start 104: t_ipv4_server_dupl
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
102/162 Test #104: t_ipv4_server_dupl ...............***Not Run   0.00 sec
        Start 105: t_icS_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
103/162 Test #105: t_icS_1M_32k .....................***Not Run   0.00 sec
        Start 106: t_icS_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
104/162 Test #106: t_icS_128K_8K ....................***Not Run   0.00 sec
        Start 107: t_icS_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
105/162 Test #107: t_icS_16_1 .......................***Not Run   0.00 sec
        Start 108: t_icS_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
106/162 Test #108: t_icS_32K_d ......................***Not Run   0.00 sec
        Start 109: t_icR_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
107/162 Test #109: t_icR_1M_32k .....................***Not Run   0.00 sec
        Start 110: t_icR_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
108/162 Test #110: t_icR_128K_8K ....................***Not Run   0.00 sec
        Start 111: t_icR_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
109/162 Test #111: t_icR_16_1 .......................***Not Run   0.00 sec
        Start 112: t_icR_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
110/162 Test #112: t_icR_32K_d ......................***Not Run   0.00 sec
        Start 113: t_icD_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
111/162 Test #113: t_icD_1M_32k .....................***Not Run   0.00 sec
        Start 114: t_icD_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
112/162 Test #114: t_icD_128K_8K ....................***Not Run   0.00 sec
        Start 115: t_icD_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
113/162 Test #115: t_icD_16_1 .......................***Not Run   0.00 sec
        Start 116: t_icD_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
114/162 Test #116: t_icD_32K_d ......................***Not Run   0.00 sec
        Start 117: t_isS_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
115/162 Test #117: t_isS_1M_32k .....................***Not Run   0.00 sec
        Start 118: t_isS_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
116/162 Test #118: t_isS_128K_8K ....................***Not Run   0.00 sec
        Start 119: t_isS_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
117/162 Test #119: t_isS_16_1 .......................***Not Run   0.00 sec
        Start 120: t_isS_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
118/162 Test #120: t_isS_32K_d ......................***Not Run   0.00 sec
        Start 121: t_isR_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
119/162 Test #121: t_isR_1M_32k .....................***Not Run   0.00 sec
        Start 122: t_isR_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
120/162 Test #122: t_isR_128K_8K ....................***Not Run   0.00 sec
        Start 123: t_isR_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
121/162 Test #123: t_isR_16_1 .......................***Not Run   0.00 sec
        Start 124: t_isR_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
122/162 Test #124: t_isR_32K_d ......................***Not Run   0.00 sec
        Start 125: t_isD_1M_32k
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
123/162 Test #125: t_isD_1M_32k .....................***Not Run   0.00 sec
        Start 126: t_isD_128K_8K
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
124/162 Test #126: t_isD_128K_8K ....................***Not Run   0.00 sec
        Start 127: t_isD_16_1
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
125/162 Test #127: t_isD_16_1 .......................***Not Run   0.00 sec
        Start 128: t_isD_32K_d
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
126/162 Test #128: t_isD_32K_d ......................***Not Run   0.00 sec
        Start 129: t_icS_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
127/162 Test #129: t_icS_128K_8K_l ..................***Not Run   0.00 sec
        Start 130: t_icS_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
128/162 Test #130: t_icS_128K_8K_L ..................***Not Run   0.00 sec
        Start 131: t_icS_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
129/162 Test #131: t_icS_128K_8K_lL .................***Not Run   0.00 sec
        Start 132: t_icR_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
130/162 Test #132: t_icR_128K_8K_l ..................***Not Run   0.00 sec
        Start 133: t_icR_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
131/162 Test #133: t_icR_128K_8K_L ..................***Not Run   0.00 sec
        Start 134: t_icR_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
132/162 Test #134: t_icR_128K_8K_lL .................***Not Run   0.00 sec
        Start 135: t_icD_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
133/162 Test #135: t_icD_128K_8K_l ..................***Not Run   0.00 sec
        Start 136: t_icD_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
134/162 Test #136: t_icD_128K_8K_L ..................***Not Run   0.00 sec
        Start 137: t_icD_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
135/162 Test #137: t_icD_128K_8K_lL .................***Not Run   0.00 sec
        Start 138: t_isS_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
136/162 Test #138: t_isS_128K_8K_l ..................***Not Run   0.00 sec
        Start 139: t_isS_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
137/162 Test #139: t_isS_128K_8K_L ..................***Not Run   0.00 sec
        Start 140: t_isS_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
138/162 Test #140: t_isS_128K_8K_lL .................***Not Run   0.00 sec
        Start 141: t_isR_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
139/162 Test #141: t_isR_128K_8K_l ..................***Not Run   0.00 sec
        Start 142: t_isR_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
140/162 Test #142: t_isR_128K_8K_L ..................***Not Run   0.00 sec
        Start 143: t_isR_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
141/162 Test #143: t_isR_128K_8K_lL .................***Not Run   0.00 sec
        Start 144: t_isD_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
142/162 Test #144: t_isD_128K_8K_l ..................***Not Run   0.00 sec
        Start 145: t_isD_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
143/162 Test #145: t_isD_128K_8K_L ..................***Not Run   0.00 sec
        Start 146: t_isD_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
144/162 Test #146: t_isD_128K_8K_lL .................***Not Run   0.00 sec
        Start 147: t_icnS_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
145/162 Test #147: t_icnS_128K_8K_l .................***Not Run   0.00 sec
        Start 148: t_icnS_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
146/162 Test #148: t_icnS_128K_8K_L .................***Not Run   0.00 sec
        Start 149: t_icnS_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
147/162 Test #149: t_icnS_128K_8K_lL ................***Not Run   0.00 sec
        Start 150: t_icnR_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
148/162 Test #150: t_icnR_128K_8K_l .................***Not Run   0.00 sec
        Start 151: t_icnR_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
149/162 Test #151: t_icnR_128K_8K_L .................***Not Run   0.00 sec
        Start 152: t_icnR_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
150/162 Test #152: t_icnR_128K_8K_lL ................***Not Run   0.00 sec
        Start 153: t_icnD_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
151/162 Test #153: t_icnD_128K_8K_l .................***Not Run   0.00 sec
        Start 154: t_icnD_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
152/162 Test #154: t_icnD_128K_8K_L .................***Not Run   0.00 sec
        Start 155: t_icnD_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
153/162 Test #155: t_icnD_128K_8K_lL ................***Not Run   0.00 sec
        Start 156: t_isnS_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
154/162 Test #156: t_isnS_128K_8K_l .................***Not Run   0.00 sec
        Start 157: t_isnS_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
155/162 Test #157: t_isnS_128K_8K_L .................***Not Run   0.00 sec
        Start 158: t_isnS_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
156/162 Test #158: t_isnS_128K_8K_lL ................***Not Run   0.00 sec
        Start 159: t_isnR_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
157/162 Test #159: t_isnR_128K_8K_l .................***Not Run   0.00 sec
        Start 160: t_isnR_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
158/162 Test #160: t_isnR_128K_8K_L .................***Not Run   0.00 sec
        Start 161: t_isnR_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
159/162 Test #161: t_isnR_128K_8K_lL ................***Not Run   0.00 sec
        Start 162: t_isnD_128K_8K_l
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
160/162 Test #162: t_isnD_128K_8K_l .................***Not Run   0.00 sec
        Start 163: t_isnD_128K_8K_L
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
161/162 Test #163: t_isnD_128K_8K_L .................***Not Run   0.00 sec
        Start 164: t_isnD_128K_8K_lL
Process not started
 /home/cs144/computer_network/sponge/txrx.sh
[permission denied]
162/162 Test #164: t_isnD_128K_8K_lL ................***Not Run   0.00 sec

25% tests passed, 121 tests failed out of 162

Total Test time (real) =   7.91 sec

The following tests FAILED:
         36 - t_active_close (Failed)
         37 - t_passive_close (Failed)
         38 - t_ack_rst (Failed)
         40 - t_connect (Failed)
         41 - t_listen (Failed)
         42 - t_winsize (Failed)
         43 - t_retx (Failed)
         51 - t_udp_client_send (BAD_COMMAND)
         52 - t_udp_server_send (BAD_COMMAND)
         53 - t_udp_client_recv (BAD_COMMAND)
         54 - t_udp_server_recv (BAD_COMMAND)
         55 - t_udp_client_dupl (BAD_COMMAND)
         56 - t_udp_server_dupl (BAD_COMMAND)
         57 - t_ucS_1M_32k (BAD_COMMAND)
         58 - t_ucS_128K_8K (BAD_COMMAND)
         59 - t_ucS_16_1 (BAD_COMMAND)
         60 - t_ucS_32K_d (BAD_COMMAND)
         61 - t_ucR_1M_32k (BAD_COMMAND)
         62 - t_ucR_128K_8K (BAD_COMMAND)
         63 - t_ucR_16_1 (BAD_COMMAND)
         64 - t_ucR_32K_d (BAD_COMMAND)
         65 - t_ucD_1M_32k (BAD_COMMAND)
         66 - t_ucD_128K_8K (BAD_COMMAND)
         67 - t_ucD_16_1 (BAD_COMMAND)
         68 - t_ucD_32K_d (BAD_COMMAND)
         69 - t_usS_1M_32k (BAD_COMMAND)
         70 - t_usS_128K_8K (BAD_COMMAND)
         71 - t_usS_16_1 (BAD_COMMAND)
         72 - t_usS_32K_d (BAD_COMMAND)
         73 - t_usR_1M_32k (BAD_COMMAND)
         74 - t_usR_128K_8K (BAD_COMMAND)
         75 - t_usR_16_1 (BAD_COMMAND)
         76 - t_usR_32K_d (BAD_COMMAND)
         77 - t_usD_1M_32k (BAD_COMMAND)
         78 - t_usD_128K_8K (BAD_COMMAND)
         79 - t_usD_16_1 (BAD_COMMAND)
         80 - t_usD_32K_d (BAD_COMMAND)
         81 - t_ucS_128K_8K_l (BAD_COMMAND)
         82 - t_ucS_128K_8K_L (BAD_COMMAND)
         83 - t_ucS_128K_8K_lL (BAD_COMMAND)
         84 - t_ucR_128K_8K_l (BAD_COMMAND)
         85 - t_ucR_128K_8K_L (BAD_COMMAND)
         86 - t_ucR_128K_8K_lL (BAD_COMMAND)
         87 - t_ucD_128K_8K_l (BAD_COMMAND)
         88 - t_ucD_128K_8K_L (BAD_COMMAND)
         89 - t_ucD_128K_8K_lL (BAD_COMMAND)
         90 - t_usS_128K_8K_l (BAD_COMMAND)
         91 - t_usS_128K_8K_L (BAD_COMMAND)
         92 - t_usS_128K_8K_lL (BAD_COMMAND)
         93 - t_usR_128K_8K_l (BAD_COMMAND)
         94 - t_usR_128K_8K_L (BAD_COMMAND)
         95 - t_usR_128K_8K_lL (BAD_COMMAND)
         96 - t_usD_128K_8K_l (BAD_COMMAND)
         97 - t_usD_128K_8K_L (BAD_COMMAND)
         98 - t_usD_128K_8K_lL (BAD_COMMAND)
         99 - t_ipv4_client_send (BAD_COMMAND)
        100 - t_ipv4_server_send (BAD_COMMAND)
        101 - t_ipv4_client_recv (BAD_COMMAND)
        102 - t_ipv4_server_recv (BAD_COMMAND)
        103 - t_ipv4_client_dupl (BAD_COMMAND)
        104 - t_ipv4_server_dupl (BAD_COMMAND)
        105 - t_icS_1M_32k (BAD_COMMAND)
        106 - t_icS_128K_8K (BAD_COMMAND)
        107 - t_icS_16_1 (BAD_COMMAND)
        108 - t_icS_32K_d (BAD_COMMAND)
        109 - t_icR_1M_32k (BAD_COMMAND)
        110 - t_icR_128K_8K (BAD_COMMAND)
        111 - t_icR_16_1 (BAD_COMMAND)
        112 - t_icR_32K_d (BAD_COMMAND)
        113 - t_icD_1M_32k (BAD_COMMAND)
        114 - t_icD_128K_8K (BAD_COMMAND)
        115 - t_icD_16_1 (BAD_COMMAND)
        116 - t_icD_32K_d (BAD_COMMAND)
        117 - t_isS_1M_32k (BAD_COMMAND)
        118 - t_isS_128K_8K (BAD_COMMAND)
        119 - t_isS_16_1 (BAD_COMMAND)
        120 - t_isS_32K_d (BAD_COMMAND)
        121 - t_isR_1M_32k (BAD_COMMAND)
        122 - t_isR_128K_8K (BAD_COMMAND)
        123 - t_isR_16_1 (BAD_COMMAND)
        124 - t_isR_32K_d (BAD_COMMAND)
        125 - t_isD_1M_32k (BAD_COMMAND)
        126 - t_isD_128K_8K (BAD_COMMAND)
        127 - t_isD_16_1 (BAD_COMMAND)
        128 - t_isD_32K_d (BAD_COMMAND)
        129 - t_icS_128K_8K_l (BAD_COMMAND)
        130 - t_icS_128K_8K_L (BAD_COMMAND)
        131 - t_icS_128K_8K_lL (BAD_COMMAND)
        132 - t_icR_128K_8K_l (BAD_COMMAND)
        133 - t_icR_128K_8K_L (BAD_COMMAND)
        134 - t_icR_128K_8K_lL (BAD_COMMAND)
        135 - t_icD_128K_8K_l (BAD_COMMAND)
        136 - t_icD_128K_8K_L (BAD_COMMAND)
        137 - t_icD_128K_8K_lL (BAD_COMMAND)
        138 - t_isS_128K_8K_l (BAD_COMMAND)
        139 - t_isS_128K_8K_L (BAD_COMMAND)
        140 - t_isS_128K_8K_lL (BAD_COMMAND)
        141 - t_isR_128K_8K_l (BAD_COMMAND)
        142 - t_isR_128K_8K_L (BAD_COMMAND)
        143 - t_isR_128K_8K_lL (BAD_COMMAND)
        144 - t_isD_128K_8K_l (BAD_COMMAND)
        145 - t_isD_128K_8K_L (BAD_COMMAND)
        146 - t_isD_128K_8K_lL (BAD_COMMAND)
        147 - t_icnS_128K_8K_l (BAD_COMMAND)
        148 - t_icnS_128K_8K_L (BAD_COMMAND)
        149 - t_icnS_128K_8K_lL (BAD_COMMAND)
        150 - t_icnR_128K_8K_l (BAD_COMMAND)
        151 - t_icnR_128K_8K_L (BAD_COMMAND)
        152 - t_icnR_128K_8K_lL (BAD_COMMAND)
        153 - t_icnD_128K_8K_l (BAD_COMMAND)
        154 - t_icnD_128K_8K_L (BAD_COMMAND)
        155 - t_icnD_128K_8K_lL (BAD_COMMAND)
        156 - t_isnS_128K_8K_l (BAD_COMMAND)
        157 - t_isnS_128K_8K_L (BAD_COMMAND)
        158 - t_isnS_128K_8K_lL (BAD_COMMAND)
        159 - t_isnR_128K_8K_l (BAD_COMMAND)
        160 - t_isnR_128K_8K_L (BAD_COMMAND)
        161 - t_isnR_128K_8K_lL (BAD_COMMAND)
        162 - t_isnD_128K_8K_l (BAD_COMMAND)
        163 - t_isnD_128K_8K_L (BAD_COMMAND)
        164 - t_isnD_128K_8K_lL (BAD_COMMAND)
Errors while running CTest
make[3]: *** [CMakeFiles/check_lab4.dir/build.make:72: CMakeFiles/check_lab4] Error 8
make[2]: *** [CMakeFiles/Makefile2:7536: CMakeFiles/check_lab4.dir/all] Error 2
make[1]: *** [CMakeFiles/Makefile2:7543: CMakeFiles/check_lab4.dir/rule] Error 2
make: *** [Makefile:2812: check_lab4] Error 2

```


## fix 1: t_active_close

### ctest for t_active_close
```bash
ctest -V -R t_active_close

UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
Test project /home/cs144/computer_network/sponge/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 36
    Start 36: t_active_close

36: Test command: /home/cs144/computer_network/sponge/build/tests/fsm_active_close
36: Working Directory: /home/cs144/computer_network/sponge/build
36: Test timeout computed to be: 10000000
36: Test Failure on expectation:
36:     Expectation: exactly one segment sent with (A=1,R=0,S=0,F=1,ackno=1,seqno=1,)
36: 
36: Failure message:
36:     The TCP should have produced a segment that existed, but it did not
36: 
36: List of steps that executed successfully:
36:     Action:      connect
36:     Expectation: exactly one segment sent with (A=0,R=0,S=1,F=0,seqno=0,payload_size=0,)
36:     Action:      packet arrives: Header(flags=SA,seqno=0,ack=1,win=137) with no payload
36:     Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=1,payload_size=0,)
36:     Action:      close
36: 
36: The TCP should have produced a segment that existed, but it did not
1/1 Test #36: t_active_close ...................***Failed    0.01 sec

0% tests passed, 1 tests failed out of 1

Total Test time (real) =   0.01 sec

The following tests FAILED:
         36 - t_active_close (Failed)
Errors while running CTest
Output from these tests are in: /home/cs144/computer_network/sponge/build/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
cs144@cs144vm:~/computer_network/sponge/build$ 
```




### GDB debug for fsm_active_close

```bash
gdb ./tests/fsm_active_close

(gdb) catch throw
# run后会出现： Catchpoint 1 (exception thrown)
(gdb) run

Starting program: /home/cs144/computer_network/sponge/build/tests/fsm_active_close 

This GDB supports auto-downloading debuginfo from the following URLs:
  <https://debuginfod.ubuntu.com>
Enable debuginfod for this session? (y or [n]) y
Debuginfod has been enabled.
To make this setting permanent, add 'set debuginfod enabled on' to .gdbinit.
Function(s) ^std::(move|forward|as_const|(__)?addressof) will be skipped when stepping.
Function(s) ^std::(shared|unique)_ptr<.*>::(get|operator) will be skipped when stepping.
Function(s) ^std::(basic_string|vector|array|deque|(forward_)?list|(unordered_|flat_)?(multi)?(map|set)|span)<.*>::(c?r?(begin|end)|front|back|data|size|empty) will be skipped when stepping.
Function(s) ^std::(basic_string|vector|array|deque|span)<.*>::operator.] will be skipped when stepping.
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Catchpoint 1 (exception thrown), 0x00007ffff7cc132a in __cxa_throw () from /lib/x86_64-linux-gnu/libstdc++.so.6

# 先看你的调用栈：
(gdb) bt
#0  0x00007ffff7cc132a in __cxa_throw () from /lib/x86_64-linux-gnu/libstdc++.so.6
#1  0x000055555556df6c in ExpectSegment::expect_seg (this=0x7fffffffd460, harness=...) at /usr/include/c++/14/bits/new_allocator.h:104
#2  0x000055555556e9f9 in ExpectOneSegment::expect_seg (this=<optimized out>, harness=...)
    at /home/cs144/computer_network/sponge/tests/tcp_expectation.hh:297
#3  0x000055555556ce39 in ExpectSegment::execute (this=<optimized out>, harness=...) at /home/cs144/computer_network/sponge/tests/tcp_expectation.hh:288
#4  0x000055555556ce84 in ExpectOneSegment::execute (this=<optimized out>, harness=...)
    at /home/cs144/computer_network/sponge/tests/tcp_expectation.hh:306
#5  0x000055555556f5f5 in TCPTestHarness::execute (this=this@entry=0x7fffffffd760, step=..., note="")
    at /home/cs144/computer_network/sponge/tests/tcp_fsm_test_harness.cc:168
#6  0x000055555557105a in TCPTestHarness::in_fin_wait_1 (cfg=..., tx_isn=..., rx_isn=...)
    at /home/cs144/computer_network/sponge/tests/tcp_expectation.hh:164
#7  0x0000555555571451 in TCPTestHarness::in_time_wait (cfg=..., tx_isn=tx_isn@entry=..., rx_isn=rx_isn@entry=...)
    at /home/cs144/computer_network/sponge/tests/tcp_fsm_test_harness.cc:358
#8  0x0000555555566f2a in main () at /home/cs144/computer_network/sponge/libsponge/wrapping_integers.hh:17

含义：
main()
  ↓
TCPTestHarness::execute()
  ↓
ExpectOneSegment::execute()
  ↓
ExpectSegment::execute()
  ↓
ExpectOneSegment::expect_seg()
  ↓
ExpectSegment::expect_seg()
  ↓
throw ...

当前停在：
#0 __cxa_throw()

即：
throw SomeException(...);
执行的那一刻。


# 然后： 调用栈中的每一层叫 frame：
#0 当前函数
#1 调用当前函数的函数
#2 调用 #1 的函数
#3 ...


(gdb) frame 1
#1  0x000055555556df6c in ExpectSegment::expect_seg (this=0x7fffffffd460, harness=...) at /usr/include/c++/14/bits/new_allocator.h:104


(gdb) list
99      #if __cplusplus >= 201103L
100           __new_allocator& operator=(const __new_allocator&) = default;
101     #endif
102
103     #if __cplusplus <= 201703L
104           ~__new_allocator() _GLIBCXX_USE_NOEXCEPT { }
105
106           pointer
107           address(reference __x) const _GLIBCXX_NOEXCEPT
108           { return std::__addressof(__x); }
(gdb) 
109
110           const_pointer
111           address(const_reference __x) const _GLIBCXX_NOEXCEPT
112           { return std::__addressof(__x); }
113     #endif
114
115     #if __has_builtin(__builtin_operator_new) >= 201802L
116     # define _GLIBCXX_OPERATOR_NEW __builtin_operator_new
117     # define _GLIBCXX_OPERATOR_DELETE __builtin_operator_delete
118     #else
(gdb) 
119     # define _GLIBCXX_OPERATOR_NEW ::operator new
120     # define _GLIBCXX_OPERATOR_DELETE ::operator delete
121     #endif
122
123           // NB: __n is permitted to be 0.  The C++ standard says nothing
124           // about what the return value is when __n == 0.
125           _GLIBCXX_NODISCARD _Tp*
126           allocate(size_type __n, const void* = static_cast<const void*>(0))
127           {
128     #if __cplusplus >= 201103L



# 切换到调用当前函数的那个 frame
# up = 往调用者方向走

(gdb) up
#2  0x000055555556e9f9 in ExpectOneSegment::expect_seg (this=<optimized out>, harness=...) at /home/cs144/computer_network/sponge/tests/tcp_expectation.hh:297
297         TCPSegment seg = ExpectSegment::expect_seg(harness);


# 当你切换到某个 frame 后 list： GDB 会打开 当前 frame 对应的源代码

(gdb) list
292       std::string description() const {
293         return "exactly one segment sent with " + segment_description();
294       }
295
296       TCPSegment expect_seg(TCPTestHarness &harness) const {
297         TCPSegment seg = ExpectSegment::expect_seg(harness);
298         if (harness.can_read()) {
299           throw SegmentExpectationViolation(
300               "The TCP an extra segment when it should not have");
301         }
(gdb) 
302         return seg;
303       }
304
305       void execute(TCPTestHarness &harness) const {
306         ExpectSegment::execute(harness);
307         if (harness.can_read()) {
308           throw SegmentExpectationViolation(
309               "The TCP an extra segment when it should not have");
310         }
311       }
(gdb) 
312     };
313
314     struct ExpectState : public TCPExpectation {
315       TCPState state;
316
317       ExpectState(TCPState stat) : state(stat) {}
318
319       std::string description() const {
320         std::ostringstream o;
321         o << "TCP in state ";
```

### recompile with Debug mode ：
```bash
# CS144 的 CMakeLists 默认类似这样：
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
# 如果 cmake .. -DCMAKE_CXX_FLAGS="-O0 -g"
# 结果变成：CMAKE_CXX_FLAGS + CMAKE_CXX_FLAGS_RELEASE
# 最终：-O0 -g -O3 -DNDEBUG 后面的 O3 赢了。

# 当前实际上是在 Release 模式 下编译，所以 GDB 出现 <optimized out> 完全正常。
cs144@cs144vm:~/computer_network/sponge/build$ grep CMAKE_BUILD_TYPE CMakeCache.txt
CMAKE_BUILD_TYPE:STRING=Release
CMAKE_BUILD_TYPE_SHADOW:STRING=Release


# -O0   完全不优化
# -Og   保留调试体验，同时做少量优化
# -O1
# -O2
# -O3
cmake .. -DCMAKE_BUILD_TYPE=Debug   -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g"

cs144@cs144vm:~/computer_network/sponge/build$ grep CMAKE_BUILD_TYPE CMakeCache.txt
CMAKE_BUILD_TYPE:STRING=Debug
CMAKE_BUILD_TYPE_SHADOW:STRING=Debug

make -j8

gdb ./tests/fsm_active_close

(gdb) catch throw
(gdb) run

```


### ctest for test #1 of t_active_close

```bash
cs144@cs144vm:~/computer_network/sponge/build$ ctest -V -R t_active_close
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
Test project /home/cs144/computer_network/sponge/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 36
    Start 36: t_active_close

36: Test command: /home/cs144/computer_network/sponge/build/tests/fsm_active_close
36: Working Directory: /home/cs144/computer_network/sponge/build
36: Test timeout computed to be: 10000000
36: Test Failure on expectation:
36:     Expectation: TCP **not** in state sender=`stream finished and fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1
36: 
36: Failure message:
36:     The TCP has `state = sender=`stream finished and fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1`, but state was expected to **not** be `sender=`stream finished and fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1`
36: 
36: List of steps that executed successfully:
36:     Action:      connect
36:     Expectation: exactly one segment sent with (A=0,R=0,S=1,F=0,seqno=0,payload_size=0,)
36:     Action:      packet arrives: Header(flags=SA,seqno=0,ack=1,win=137) with no payload
36:     Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=1,payload_size=0,)
36:     Action:      close
36:     Expectation: exactly one segment sent with (A=1,R=0,S=0,F=1,ackno=1,seqno=1,)
36:     Action:      packet arrives: Header(flags=AF,seqno=1,ack=2,win=137) with no payload
36:     Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=2,)
36:     Action:      9999ms pass
36:     Expectation: TCP in state sender=`stream finished and fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1
36:     Action:      1ms pass
36: 
36: Warning: Unclean shutdown of TCPConnection
36: The TCP has `state = sender=`stream finished and fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1`, but state was expected to **not** be `sender=`stream finished and fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1`
36: /home/cs144/computer_network/sponge/build/tests/fsm_active_close(+0xbe6b) [0x5a7d44b92e6b]
36: /lib/x86_64-linux-gnu/libc.so.6(+0x2a578) [0x742c69c2a578]
36: /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x8b) [0x742c69c2a63b]
36: /home/cs144/computer_network/sponge/build/tests/fsm_active_close(_start+0x25) [0x5a7d44b98ef5]
1/1 Test #36: t_active_close ...................***Failed    0.02 sec

0% tests passed, 1 tests failed out of 1

Total Test time (real) =   0.03 sec

The following tests FAILED:
         36 - t_active_close (Failed)
Errors while running CTest
Output from these tests are in: /home/cs144/computer_network/sponge/build/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
```


### ctest for test #2 of t_active_close

```bash
cs144@cs144vm:~/computer_network/sponge/build$ ctest -V -R t_active_close
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
Test project /home/cs144/computer_network/sponge/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 36
    Start 36: t_active_close

36: Test command: /home/cs144/computer_network/sponge/build/tests/fsm_active_close
36: Working Directory: /home/cs144/computer_network/sponge/build
36: Test timeout computed to be: 10000000
36: Test Failure on expectation:
36:     Expectation: TCP in state sender=`stream finished (FIN sent) but not fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1
36: 
36: Failure message:
36:     The TCP was in state `sender=`stream finished (FIN sent) but not fully acknowledged`, receiver=`input to stream has ended`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`stream finished (FIN sent) but not fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1`
36: 
36: List of steps that executed successfully:
36:     Action:      connect
36:     Expectation: exactly one segment sent with (A=0,R=0,S=1,F=0,seqno=0,payload_size=0,)
36:     Action:      packet arrives: Header(flags=SA,seqno=0,ack=1,win=137) with no payload
36:     Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=1,payload_size=0,)
36:     Action:      close
36:     Expectation: exactly one segment sent with (A=1,R=0,S=0,F=1,ackno=1,seqno=1,)
36:     Action:      packet arrives: Header(flags=AF,seqno=1,ack=1,win=137) with no payload
36:     Expectation: exactly one segment sent with (A=1,R=0,S=0,F=0,ackno=2,)
36:     Action:      4000ms pass
36:     Expectation: exactly one segment sent with (F=1,)
36: 
36: The TCP was in state `sender=`stream finished (FIN sent) but not fully acknowledged`, receiver=`input to stream has ended`, active=0, linger_after_streams_finish=0`, but it was expected to be in state `sender=`stream finished (FIN sent) but not fully acknowledged`, receiver=`input to stream has ended`, active=1, linger_after_streams_finish=1`
36: /home/cs144/computer_network/sponge/build/tests/fsm_active_close(+0xbe6b) [0x6429fcd7ce6b]
36: /lib/x86_64-linux-gnu/libc.so.6(+0x2a578) [0x7c124242a578]
36: /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x8b) [0x7c124242a63b]
36: /home/cs144/computer_network/sponge/build/tests/fsm_active_close(_start+0x25) [0x6429fcd82ef5]
1/1 Test #36: t_active_close ...................***Failed    0.01 sec

0% tests passed, 1 tests failed out of 1

Total Test time (real) =   0.02 sec

The following tests FAILED:
         36 - t_active_close (Failed)
Errors while running CTest
Output from these tests are in: /home/cs144/computer_network/sponge/build/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
```

#### GDB Debug test #2
```bash
# Launch GDB
gdb ./tests/fsm_active_close

#  catch any exception
(gdb) catch throw
Catchpoint 1 (throw)

#  or break whenever TCPExpectationViolation is thrown
(gdb) catch throw TCPExpectationViolation

#  or break when a specific expectation is checked
(gdb) break TCPExpectationViolation::TCPExpectationViolation

# run后会出现： Catch point 1 (exception thrown)
# This stops at the exact line that throws, before the catch block unwinds the stack.
(gdb) run

(gdb) catch throw

(gdb) run
Starting program: /home/cs144/computer_network/sponge/build/tests/fsm_active_close 

This GDB supports auto-downloading debuginfo from the following URLs:
  <https://debuginfod.ubuntu.com>
Enable debuginfod for this session? (y or [n]) y
Debuginfod has been enabled.
To make this setting permanent, add 'set debuginfod enabled on' to .gdbinit.
Function(s) ^std::(move|forward|as_const|(__)?addressof) will be skipped when stepping.
Function(s) ^std::(shared|unique)_ptr<.*>::(get|operator) will be skipped when stepping.
Function(s) ^std::(basic_string|vector|array|deque|(forward_)?list|(unordered_|flat_)?(multi)?(map|set)|span)<.*>::(c?r?(begin|end)|front|back|data|size|empty) will be skipped when stepping.
Function(s) ^std::(basic_string|vector|array|deque|span)<.*>::operator.] will be skipped when stepping.
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Catchpoint 1 (exception thrown), 0x00007ffff7cc132a in __cxa_throw () from /lib/x86_64-linux-gnu/libstdc++.so.6
(gdb) bt
#0  0x00007ffff7cc132a in __cxa_throw () from /lib/x86_64-linux-gnu/libstdc++.so.6
#1  0x000055555556c1e4 in ExpectState::execute (this=<optimized out>, harness=...) at /home/cs144/computer_network/sponge/tests/tcp_expectation.hh:329
#2  0x000055555556f5f5 in TCPTestHarness::execute (this=this@entry=0x7fffffffd760, step=..., note="") at /home/cs144/computer_network/sponge/tests/tcp_fsm_test_harness.cc:168
#3  0x000055555556744c in main () at /home/cs144/computer_network/sponge/tests/fsm_active_close.cc:51
(gdb) frame 2
#2  0x000055555556f5f5 in TCPTestHarness::execute (this=this@entry=0x7fffffffd760, step=..., note="") at /home/cs144/computer_network/sponge/tests/tcp_fsm_test_harness.cc:168
168         step.execute(*this);
(gdb) list
163                   .with_win(DEFAULT_TEST_WINDOW));
164     }
165
166     void TCPTestHarness::execute(const TCPTestStep &step, std::string note) {
167       try {
168         step.execute(*this);
169         while (not _fsm.segments_out().empty()) {
170           _flt.write(_fsm.segments_out().front());
171           _fsm.segments_out().pop();
172         }
(gdb) frame 1
#1  0x000055555556c1e4 in ExpectState::execute (this=<optimized out>, harness=...) at /home/cs144/computer_network/sponge/tests/tcp_expectation.hh:329
329           throw StateExpectationViolation{state, actual_state};
(gdb) list
324       }
325
326       void execute(TCPTestHarness &harness) const {
327         TCPState actual_state = harness._fsm.state();
328         if (actual_state != state) {
329           throw StateExpectationViolation{state, actual_state};
330         }
331       }
332     };
333
(gdb) p state
value has been optimized out
(gdb) p actual_state
$1 = {_sender = "stream finished (FIN sent) but not fully acknowledged", _receiver = "input to stream has ended", _active = false, _linger_after_streams_finish = false}
(gdb) 

```


## fix2: t_ack_rst

### ctest for t_ack_rst

```bash
cs144@cs144vm:~/computer_network/sponge/build$ ctest -V -R t_ack_rst
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
Test project /home/cs144/computer_network/sponge/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 38
    Start 38: t_ack_rst

38: Test command: /home/cs144/computer_network/sponge/build/tests/fsm_ack_rst_relaxed
38: Working Directory: /home/cs144/computer_network/sponge/build
38: Test timeout computed to be: 10000000
38: Test 1
38: Test 2
38: Test 3
38: Test Failure on expectation:
38:     Expectation: TCP in state sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1
38: 
38: Failure message:
38:     The TCP was in state `sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1`, but it was expected to be in state `sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1`
38: 
38: List of steps that executed successfully:
38:     Action:      listen
38:     Action:      packet arrives: Header(flags=A,seqno=2147483648,ack=2147483648,win=137) with no payload
38: 
38: Warning: Unclean shutdown of TCPConnection
38: The TCP was in state `sender=`stream started but nothing acknowledged`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1`, but it was expected to be in state `sender=`waiting for stream to begin (no SYN sent)`, receiver=`waiting for SYN: ackno is empty`, active=1, linger_after_streams_finish=1` (ack_listen_test called from line 131)
1/2 Test #38: t_ack_rst ........................***Failed    0.01 sec
test 39
    Start 39: t_ack_rst_win

39: Test command: /home/cs144/computer_network/sponge/build/tests/fsm_ack_rst_win_relaxed
39: Working Directory: /home/cs144/computer_network/sponge/build
39: Test timeout computed to be: 10000000
2/2 Test #39: t_ack_rst_win ....................   Passed    0.01 sec

The following tests passed:
        t_ack_rst_win

50% tests passed, 1 tests failed out of 2

Total Test time (real) =   0.04 sec

The following tests FAILED:
         38 - t_ack_rst (Failed)
Errors while running CTest
Output from these tests are in: /home/cs144/computer_network/sponge/build/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
```

### Vscode debug for t_ack_rst


### GDB Debug for t_ack_rst

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug   -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g -fno-inline -fno-omit-frame-pointer"
# -O0                     disable all optimization
# -g                      full debug symbols
# -fno-inline             prevent function inlining
# -fno-omit-frame-pointer keep frame pointers for clean backtraces

make -j8

gdb ./tests/fsm_ack_rst_relaxed

(gdb) catch throw

(gdb) b fsm_ack_rst_relaxed.cc:24

(gdb) run

# When It Stops at Line 24
# confirm where you are
list

# check the harness state BEFORE execute() is called
print harness._fsm._active
print harness._fsm._linger_after_streams_finish
print harness._fsm._sender._syn_sent
print harness._fsm._sender._fin_sent
print harness._fsm._receiver._syn_received

# Step Into ExpectState::execute()

```

## 2nd overall test
```bash
cs144@cs144vm:~/computer_network/sponge/build$ make check_lab4
[100%] Testing the TCP connection...
Test project /home/cs144/computer_network/sponge/build
        Start   1: t_wrapping_ints_cmp
  1/162 Test   #1: t_wrapping_ints_cmp ..............   Passed    0.00 sec
        Start   2: t_wrapping_ints_unwrap
  2/162 Test   #2: t_wrapping_ints_unwrap ...........   Passed    0.00 sec
        Start   3: t_wrapping_ints_wrap
  3/162 Test   #3: t_wrapping_ints_wrap .............   Passed    0.00 sec
        Start   4: t_wrapping_ints_roundtrip
  4/162 Test   #4: t_wrapping_ints_roundtrip ........   Passed    0.20 sec
        Start   5: t_recv_connect
  5/162 Test   #5: t_recv_connect ...................   Passed    0.01 sec
        Start   6: t_recv_transmit
  6/162 Test   #6: t_recv_transmit ..................   Passed    0.07 sec
        Start   7: t_recv_window
  7/162 Test   #7: t_recv_window ....................   Passed    0.01 sec
        Start   8: t_recv_reorder
  8/162 Test   #8: t_recv_reorder ...................   Passed    0.01 sec
        Start   9: t_recv_close
  9/162 Test   #9: t_recv_close .....................   Passed    0.01 sec
        Start  10: t_recv_special
 10/162 Test  #10: t_recv_special ...................   Passed    0.01 sec
        Start  11: t_send_connect
 11/162 Test  #11: t_send_connect ...................   Passed    0.01 sec
        Start  12: t_send_transmit
 12/162 Test  #12: t_send_transmit ..................   Passed    0.09 sec
        Start  13: t_send_retx
 13/162 Test  #13: t_send_retx ......................   Passed    0.01 sec
        Start  14: t_send_window
 14/162 Test  #14: t_send_window ....................   Passed    0.14 sec
        Start  15: t_send_ack
 15/162 Test  #15: t_send_ack .......................   Passed    0.02 sec
        Start  16: t_send_close
 16/162 Test  #16: t_send_close .....................   Passed    0.02 sec
        Start  17: t_send_extra
 17/162 Test  #17: t_send_extra .....................   Passed    0.02 sec
        Start  18: t_strm_reassem_single
 18/162 Test  #18: t_strm_reassem_single ............   Passed    0.01 sec
        Start  19: t_strm_reassem_seq
 19/162 Test  #19: t_strm_reassem_seq ...............   Passed    0.02 sec
        Start  20: t_strm_reassem_dup
 20/162 Test  #20: t_strm_reassem_dup ...............   Passed    0.02 sec
        Start  21: t_strm_reassem_holes
 21/162 Test  #21: t_strm_reassem_holes .............   Passed    0.01 sec
        Start  22: t_strm_reassem_many
 22/162 Test  #22: t_strm_reassem_many ..............   Passed    5.83 sec
        Start  23: t_strm_reassem_overlapping
 23/162 Test  #23: t_strm_reassem_overlapping .......   Passed    0.02 sec
        Start  24: t_strm_reassem_win
 24/162 Test  #24: t_strm_reassem_win ...............   Passed    6.34 sec
        Start  25: t_strm_reassem_cap
 25/162 Test  #25: t_strm_reassem_cap ...............   Passed    0.16 sec
        Start  26: t_byte_stream_construction
 26/162 Test  #26: t_byte_stream_construction .......   Passed    0.02 sec
        Start  27: t_byte_stream_one_write
 27/162 Test  #27: t_byte_stream_one_write ..........   Passed    0.01 sec
        Start  28: t_byte_stream_two_writes
 28/162 Test  #28: t_byte_stream_two_writes .........   Passed    0.00 sec
        Start  29: t_byte_stream_capacity
 29/162 Test  #29: t_byte_stream_capacity ...........   Passed    0.63 sec
        Start  30: t_byte_stream_many_writes
 30/162 Test  #30: t_byte_stream_many_writes ........   Passed    0.03 sec
        Start  31: t_webget
 31/162 Test  #31: t_webget .........................   Passed    1.80 sec
        Start  34: t_tcp_parser
 32/162 Test  #34: t_tcp_parser .....................   Passed    0.04 sec
        Start  35: t_ipv4_parser
 33/162 Test  #35: t_ipv4_parser ....................   Passed    0.02 sec
        Start  36: t_active_close
 34/162 Test  #36: t_active_close ...................   Passed    0.02 sec
        Start  37: t_passive_close
 35/162 Test  #37: t_passive_close ..................   Passed    0.02 sec
        Start  38: t_ack_rst
 36/162 Test  #38: t_ack_rst ........................   Passed    0.02 sec
        Start  39: t_ack_rst_win
 37/162 Test  #39: t_ack_rst_win ....................   Passed    0.01 sec
        Start  40: t_connect
 38/162 Test  #40: t_connect ........................   Passed    0.02 sec
        Start  41: t_listen
 39/162 Test  #41: t_listen .........................   Passed    0.01 sec
        Start  42: t_winsize
 40/162 Test  #42: t_winsize ........................   Passed    0.18 sec
        Start  43: t_retx
 41/162 Test  #43: t_retx ...........................   Passed    0.01 sec
        Start  44: t_retx_win
 42/162 Test  #44: t_retx_win .......................   Passed    0.01 sec
        Start  45: t_loopback
 43/162 Test  #45: t_loopback .......................   Passed    4.42 sec
        Start  46: t_loopback_win
 44/162 Test  #46: t_loopback_win ...................   Passed    3.00 sec
        Start  47: t_reorder
 45/162 Test  #47: t_reorder ........................   Passed    5.77 sec
        Start  48: t_address_dt
 46/162 Test  #48: t_address_dt .....................   Passed    0.07 sec
        Start  49: t_parser_dt
 47/162 Test  #49: t_parser_dt ......................   Passed    0.01 sec
        Start  50: t_socket_dt
 48/162 Test  #50: t_socket_dt ......................   Passed    0.01 sec
        Start  51: t_udp_client_send
 49/162 Test  #51: t_udp_client_send ................   Passed    0.32 sec
        Start  52: t_udp_server_send
 50/162 Test  #52: t_udp_server_send ................   Passed    0.33 sec
        Start  53: t_udp_client_recv
 51/162 Test  #53: t_udp_client_recv ................   Passed    0.31 sec
        Start  54: t_udp_server_recv
 52/162 Test  #54: t_udp_server_recv ................   Passed    0.31 sec
        Start  55: t_udp_client_dupl
 53/162 Test  #55: t_udp_client_dupl ................   Passed    0.33 sec
        Start  56: t_udp_server_dupl
 54/162 Test  #56: t_udp_server_dupl ................   Passed    0.33 sec
        Start  57: t_ucS_1M_32k
 55/162 Test  #57: t_ucS_1M_32k .....................   Passed    1.68 sec
        Start  58: t_ucS_128K_8K
 56/162 Test  #58: t_ucS_128K_8K ....................   Passed    0.51 sec
        Start  59: t_ucS_16_1
 57/162 Test  #59: t_ucS_16_1 .......................   Passed    0.36 sec
        Start  60: t_ucS_32K_d
 58/162 Test  #60: t_ucS_32K_d ......................   Passed    0.35 sec
        Start  61: t_ucR_1M_32k
 59/162 Test  #61: t_ucR_1M_32k .....................   Passed    1.67 sec
        Start  62: t_ucR_128K_8K
 60/162 Test  #62: t_ucR_128K_8K ....................   Passed    0.50 sec
        Start  63: t_ucR_16_1
 61/162 Test  #63: t_ucR_16_1 .......................   Passed    0.34 sec
        Start  64: t_ucR_32K_d
 62/162 Test  #64: t_ucR_32K_d ......................   Passed    0.40 sec
        Start  65: t_ucD_1M_32k
 63/162 Test  #65: t_ucD_1M_32k .....................   Passed    1.71 sec
        Start  66: t_ucD_128K_8K
 64/162 Test  #66: t_ucD_128K_8K ....................   Passed    0.52 sec
        Start  67: t_ucD_16_1
 65/162 Test  #67: t_ucD_16_1 .......................   Passed    0.36 sec
        Start  68: t_ucD_32K_d
 66/162 Test  #68: t_ucD_32K_d ......................   Passed    0.37 sec
        Start  69: t_usS_1M_32k
 67/162 Test  #69: t_usS_1M_32k .....................   Passed    1.65 sec
        Start  70: t_usS_128K_8K
 68/162 Test  #70: t_usS_128K_8K ....................   Passed    0.47 sec
        Start  71: t_usS_16_1
 69/162 Test  #71: t_usS_16_1 .......................   Passed    0.32 sec
        Start  72: t_usS_32K_d
 70/162 Test  #72: t_usS_32K_d ......................   Passed    0.36 sec
        Start  73: t_usR_1M_32k
 71/162 Test  #73: t_usR_1M_32k .....................   Passed    1.52 sec
        Start  74: t_usR_128K_8K
 72/162 Test  #74: t_usR_128K_8K ....................   Passed    0.48 sec
        Start  75: t_usR_16_1
 73/162 Test  #75: t_usR_16_1 .......................   Passed    0.35 sec
        Start  76: t_usR_32K_d
 74/162 Test  #76: t_usR_32K_d ......................   Passed    0.38 sec
        Start  77: t_usD_1M_32k
 75/162 Test  #77: t_usD_1M_32k .....................   Passed    1.76 sec
        Start  78: t_usD_128K_8K
 76/162 Test  #78: t_usD_128K_8K ....................   Passed    0.53 sec
        Start  79: t_usD_16_1
 77/162 Test  #79: t_usD_16_1 .......................   Passed    0.35 sec
        Start  80: t_usD_32K_d
 78/162 Test  #80: t_usD_32K_d ......................   Passed    0.39 sec
        Start  81: t_ucS_128K_8K_l
 79/162 Test  #81: t_ucS_128K_8K_l ..................   Passed    0.51 sec
        Start  82: t_ucS_128K_8K_L
 80/162 Test  #82: t_ucS_128K_8K_L ..................   Passed    0.72 sec
        Start  83: t_ucS_128K_8K_lL
 81/162 Test  #83: t_ucS_128K_8K_lL .................   Passed    0.80 sec
        Start  84: t_ucR_128K_8K_l
 82/162 Test  #84: t_ucR_128K_8K_l ..................   Passed    0.76 sec
        Start  85: t_ucR_128K_8K_L
 83/162 Test  #85: t_ucR_128K_8K_L ..................   Passed    0.52 sec
        Start  86: t_ucR_128K_8K_lL
 84/162 Test  #86: t_ucR_128K_8K_lL .................   Passed    0.86 sec
        Start  87: t_ucD_128K_8K_l
 85/162 Test  #87: t_ucD_128K_8K_l ..................   Passed    0.68 sec
        Start  88: t_ucD_128K_8K_L
 86/162 Test  #88: t_ucD_128K_8K_L ..................   Passed    0.77 sec
        Start  89: t_ucD_128K_8K_lL
 87/162 Test  #89: t_ucD_128K_8K_lL .................   Passed    1.04 sec
        Start  90: t_usS_128K_8K_l
 88/162 Test  #90: t_usS_128K_8K_l ..................   Passed    0.49 sec
        Start  91: t_usS_128K_8K_L
 89/162 Test  #91: t_usS_128K_8K_L ..................   Passed    0.62 sec
        Start  92: t_usS_128K_8K_lL
 90/162 Test  #92: t_usS_128K_8K_lL .................   Passed    0.76 sec
        Start  93: t_usR_128K_8K_l
 91/162 Test  #93: t_usR_128K_8K_l ..................   Passed    0.68 sec
        Start  94: t_usR_128K_8K_L
 92/162 Test  #94: t_usR_128K_8K_L ..................   Passed    0.50 sec
        Start  95: t_usR_128K_8K_lL
 93/162 Test  #95: t_usR_128K_8K_lL .................   Passed    0.65 sec
        Start  96: t_usD_128K_8K_l
 94/162 Test  #96: t_usD_128K_8K_l ..................   Passed    0.79 sec
        Start  97: t_usD_128K_8K_L
 95/162 Test  #97: t_usD_128K_8K_L ..................   Passed    0.85 sec
        Start  98: t_usD_128K_8K_lL
 96/162 Test  #98: t_usD_128K_8K_lL .................   Passed    0.91 sec
        Start  99: t_ipv4_client_send
 97/162 Test  #99: t_ipv4_client_send ...............   Passed    0.32 sec
        Start 100: t_ipv4_server_send
 98/162 Test #100: t_ipv4_server_send ...............   Passed    0.33 sec
        Start 101: t_ipv4_client_recv
 99/162 Test #101: t_ipv4_client_recv ...............   Passed    0.35 sec
        Start 102: t_ipv4_server_recv
100/162 Test #102: t_ipv4_server_recv ...............   Passed    0.38 sec
        Start 103: t_ipv4_client_dupl
101/162 Test #103: t_ipv4_client_dupl ...............   Passed    0.33 sec
        Start 104: t_ipv4_server_dupl
102/162 Test #104: t_ipv4_server_dupl ...............   Passed    0.37 sec
        Start 105: t_icS_1M_32k
103/162 Test #105: t_icS_1M_32k .....................   Passed    1.68 sec
        Start 106: t_icS_128K_8K
104/162 Test #106: t_icS_128K_8K ....................   Passed    0.49 sec
        Start 107: t_icS_16_1
105/162 Test #107: t_icS_16_1 .......................   Passed    0.35 sec
        Start 108: t_icS_32K_d
106/162 Test #108: t_icS_32K_d ......................   Passed    0.37 sec
        Start 109: t_icR_1M_32k
107/162 Test #109: t_icR_1M_32k .....................   Passed    1.65 sec
        Start 110: t_icR_128K_8K
108/162 Test #110: t_icR_128K_8K ....................   Passed    0.51 sec
        Start 111: t_icR_16_1
109/162 Test #111: t_icR_16_1 .......................   Passed    0.36 sec
        Start 112: t_icR_32K_d
110/162 Test #112: t_icR_32K_d ......................   Passed    0.36 sec
        Start 113: t_icD_1M_32k
111/162 Test #113: t_icD_1M_32k .....................   Passed    1.83 sec
        Start 114: t_icD_128K_8K
112/162 Test #114: t_icD_128K_8K ....................   Passed    0.54 sec
        Start 115: t_icD_16_1
113/162 Test #115: t_icD_16_1 .......................   Passed    0.36 sec
        Start 116: t_icD_32K_d
114/162 Test #116: t_icD_32K_d ......................   Passed    0.38 sec
        Start 117: t_isS_1M_32k
115/162 Test #117: t_isS_1M_32k .....................   Passed    1.79 sec
        Start 118: t_isS_128K_8K
116/162 Test #118: t_isS_128K_8K ....................   Passed    0.49 sec
        Start 119: t_isS_16_1
117/162 Test #119: t_isS_16_1 .......................   Passed    0.35 sec
        Start 120: t_isS_32K_d
118/162 Test #120: t_isS_32K_d ......................   Passed    0.38 sec
        Start 121: t_isR_1M_32k
119/162 Test #121: t_isR_1M_32k .....................   Passed    1.68 sec
        Start 122: t_isR_128K_8K
120/162 Test #122: t_isR_128K_8K ....................   Passed    0.49 sec
        Start 123: t_isR_16_1
121/162 Test #123: t_isR_16_1 .......................   Passed    0.36 sec
        Start 124: t_isR_32K_d
122/162 Test #124: t_isR_32K_d ......................   Passed    0.37 sec
        Start 125: t_isD_1M_32k
123/162 Test #125: t_isD_1M_32k .....................   Passed    1.87 sec
        Start 126: t_isD_128K_8K
124/162 Test #126: t_isD_128K_8K ....................   Passed    0.55 sec
        Start 127: t_isD_16_1
125/162 Test #127: t_isD_16_1 .......................   Passed    0.34 sec
        Start 128: t_isD_32K_d
126/162 Test #128: t_isD_32K_d ......................   Passed    0.39 sec
        Start 129: t_icS_128K_8K_l
127/162 Test #129: t_icS_128K_8K_l ..................   Passed    0.49 sec
        Start 130: t_icS_128K_8K_L
128/162 Test #130: t_icS_128K_8K_L ..................   Passed    0.70 sec
        Start 131: t_icS_128K_8K_lL
129/162 Test #131: t_icS_128K_8K_lL .................   Passed    0.84 sec
        Start 132: t_icR_128K_8K_l
130/162 Test #132: t_icR_128K_8K_l ..................   Passed    0.67 sec
        Start 133: t_icR_128K_8K_L
131/162 Test #133: t_icR_128K_8K_L ..................   Passed    0.57 sec
        Start 134: t_icR_128K_8K_lL
132/162 Test #134: t_icR_128K_8K_lL .................   Passed    0.66 sec
        Start 135: t_icD_128K_8K_l
133/162 Test #135: t_icD_128K_8K_l ..................   Passed    0.69 sec
        Start 136: t_icD_128K_8K_L
134/162 Test #136: t_icD_128K_8K_L ..................   Passed    0.69 sec
        Start 137: t_icD_128K_8K_lL
135/162 Test #137: t_icD_128K_8K_lL .................   Passed    0.97 sec
        Start 138: t_isS_128K_8K_l
136/162 Test #138: t_isS_128K_8K_l ..................   Passed    0.46 sec
        Start 139: t_isS_128K_8K_L
137/162 Test #139: t_isS_128K_8K_L ..................   Passed    0.66 sec
        Start 140: t_isS_128K_8K_lL
138/162 Test #140: t_isS_128K_8K_lL .................   Passed    0.55 sec
        Start 141: t_isR_128K_8K_l
139/162 Test #141: t_isR_128K_8K_l ..................   Passed    0.89 sec
        Start 142: t_isR_128K_8K_L
140/162 Test #142: t_isR_128K_8K_L ..................   Passed    0.46 sec
        Start 143: t_isR_128K_8K_lL
141/162 Test #143: t_isR_128K_8K_lL .................   Passed    0.96 sec
        Start 144: t_isD_128K_8K_l
142/162 Test #144: t_isD_128K_8K_l ..................   Passed    0.78 sec
        Start 145: t_isD_128K_8K_L
143/162 Test #145: t_isD_128K_8K_L ..................   Passed    0.85 sec
        Start 146: t_isD_128K_8K_lL
144/162 Test #146: t_isD_128K_8K_lL .................   Passed    0.78 sec
        Start 147: t_icnS_128K_8K_l
145/162 Test #147: t_icnS_128K_8K_l .................   Passed    0.67 sec
        Start 148: t_icnS_128K_8K_L
146/162 Test #148: t_icnS_128K_8K_L .................   Passed    0.48 sec
        Start 149: t_icnS_128K_8K_lL
147/162 Test #149: t_icnS_128K_8K_lL ................   Passed    0.45 sec
        Start 150: t_icnR_128K_8K_l
148/162 Test #150: t_icnR_128K_8K_l .................   Passed    1.08 sec
        Start 151: t_icnR_128K_8K_L
149/162 Test #151: t_icnR_128K_8K_L .................   Passed    0.53 sec
        Start 152: t_icnR_128K_8K_lL
150/162 Test #152: t_icnR_128K_8K_lL ................***Timeout  10.02 sec
DEBUG: Connecting to 169.254.144.1:8701...
Successfully connected to 169.254.144.1:8701.
DEBUG: Outbound stream to 169.254.144.1:8701 finished (1 byte still in flight).
DEBUG: Outbound stream to 169.254.144.1:8701 has been fully acknowledged.

        Start 153: t_icnD_128K_8K_l
151/162 Test #153: t_icnD_128K_8K_l .................   Passed    0.80 sec
        Start 154: t_icnD_128K_8K_L
152/162 Test #154: t_icnD_128K_8K_L .................   Passed    0.58 sec
        Start 155: t_icnD_128K_8K_lL
153/162 Test #155: t_icnD_128K_8K_lL ................   Passed    0.97 sec
        Start 156: t_isnS_128K_8K_l
154/162 Test #156: t_isnS_128K_8K_l .................   Passed    0.35 sec
        Start 157: t_isnS_128K_8K_L
155/162 Test #157: t_isnS_128K_8K_L .................   Passed    0.35 sec
        Start 158: t_isnS_128K_8K_lL
156/162 Test #158: t_isnS_128K_8K_lL ................   Passed    0.37 sec
        Start 159: t_isnR_128K_8K_l
157/162 Test #159: t_isnR_128K_8K_l .................   Passed    0.79 sec
        Start 160: t_isnR_128K_8K_L
158/162 Test #160: t_isnR_128K_8K_L .................   Passed    0.55 sec
        Start 161: t_isnR_128K_8K_lL
159/162 Test #161: t_isnR_128K_8K_lL ................***Timeout  10.02 sec
DEBUG: Listening for incoming connection...
New connection from 169.254.144.1:47856.
DEBUG: Outbound stream to 169.254.144.1:47856 finished (1 byte still in flight).
DEBUG: Outbound stream to 169.254.144.1:47856 has been fully acknowledged.

        Start 162: t_isnD_128K_8K_l
160/162 Test #162: t_isnD_128K_8K_l .................   Passed    0.98 sec
        Start 163: t_isnD_128K_8K_L
161/162 Test #163: t_isnD_128K_8K_L .................   Passed    0.53 sec
        Start 164: t_isnD_128K_8K_lL
162/162 Test #164: t_isnD_128K_8K_lL ................   Passed    1.30 sec

99% tests passed, 2 tests failed out of 162

Total Test time (real) = 124.94 sec

The following tests FAILED:
        152 - t_icnR_128K_8K_lL (Timeout)
        161 - t_isnR_128K_8K_lL (Timeout)
Errors while running CTest
make[3]: *** [CMakeFiles/check_lab4.dir/build.make:72: CMakeFiles/check_lab4] Error 8
make[2]: *** [CMakeFiles/Makefile2:7536: CMakeFiles/check_lab4.dir/all] Error 2
make[1]: *** [CMakeFiles/Makefile2:7543: CMakeFiles/check_lab4.dir/rule] Error 2
make: *** [Makefile:2812: check_lab4] Error 2

```

### ctest for t_icnR_128K_8K_lL

```bash
cs144@cs144vm:~/computer_network/sponge/build$ ctest -V -R t_icnR_128K_8K_lL
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
Test project /home/cs144/computer_network/sponge/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 152
    Start 152: t_icnR_128K_8K_lL

152: Test command: /home/cs144/computer_network/sponge/txrx.sh "-icRnd" "128K" "-w" "8K" "-l" "0.1" "-L" "0.1"
152: Working Directory: /home/cs144/computer_network/sponge/build
152: Test timeout computed to be: 10000000
152: DEBUG: Connecting to 169.254.144.1:1725...
152: Successfully connected to 169.254.144.1:1725.
152: DEBUG: Outbound stream to 169.254.144.1:1725 finished (1 byte still in flight).
152: DEBUG: Outbound stream to 169.254.144.1:1725 has been fully acknowledged.
152: DEBUG: Inbound stream from 169.254.144.1:1725 finished cleanly.
152: DEBUG: Waiting for lingering segments (e.g. retransmissions of FIN) from peer...
152: DEBUG: Waiting for clean shutdown... DEBUG: TCP connection finished cleanly.
152: done.
1/1 Test #152: t_icnR_128K_8K_lL ................   Passed    0.98 sec

The following tests passed:
        t_icnR_128K_8K_lL

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.99 sec

```


## Play around with your TCP 

### Sever side
```bash
cs144@cs144vm:~/computer_network/sponge/build$ ./apps/tcp_udp -l 127.0.0.1 9090
DEBUG: Listening for incoming connection...
New connection from 127.0.0.1:47510.
I am michael
how can I help you
DEBUG: Inbound stream from 127.0.0.1:47510 finished cleanly.
DEBUG: Waiting for clean shutdown... DEBUG: Outbound stream to 127.0.0.1:47510 finished (1 byte still in flight).
DEBUG: Outbound stream to 127.0.0.1:47510 has been fully acknowledged.
DEBUG: TCP connection finished cleanly.
done.
```

### Client Side

```bash
cs144@cs144vm:~/computer_network/sponge/build$ ./apps/tcp_udp 127.0.0.1 9090
DEBUG: Connecting to 127.0.0.1:9090...
Successfully connected to 127.0.0.1:9090.
I am michael
how can I help you
DEBUG: Outbound stream to 127.0.0.1:9090 finished (1 byte still in flight).
DEBUG: Outbound stream to 127.0.0.1:9090 has been fully acknowledged.
DEBUG: Inbound stream from 127.0.0.1:9090 finished cleanly.
DEBUG: Waiting for lingering segments (e.g. retransmissions of FIN) from peer...
DEBUG: Waiting for clean shutdown... DEBUG: TCP connection finished cleanly.
done.
```