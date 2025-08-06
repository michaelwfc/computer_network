
# Why am I doing this? 

Providing a service or an abstraction on top of a different less-reliable service accounts for many of the interesting problems in networking. Over the last 40 years, researchers and practitioners have figured out how to convey all kinds of things--- messaging and e-mail, hyperlinked documents, search engines, sound and
video, virtual worlds, collaborative file sharing, digital currencies over the Internet.

TCP's own role, providing a pair of reliable byte streams using unreliable datagrams, is one of the classic examples of this. A reasonable view has it that TCP implementations count as the most widely used nontrivial computer programs on the planet.


# 1. Overview

1. In Lab 1, you’ll implement a stream reassembler—a module that stitches small pieces of the byte stream (known as substrings, or segments) back into a contiguous stream of bytes in the correct sequence. 
2. In Lab 2, you’ll implement the part of TCP that handles the inbound byte-stream: the TCPReceiver. 
   This involves thinking about how TCP will represent each byte’s place in the stream—known as a “sequence number.” The TCPReceiver is responsible for telling the sender 
   (a) how much of the inbound byte stream it’s been able to assemble successfully (this is called “acknowledgment”) 
   (b) how many more bytes the sender is allowed to send right now (“flow control”). 
3. In Lab 3, you’ll implement the part of TCP that handles the outbound byte-stream: the TCPSender. 
   How should the sender react when it suspects that a segment it transmitted was lost along the way and never made it to the receiver?   
   When should it try again and re-transmit a lost segment? 
4. In Lab 4, you’ll combine your work from the previous to labs to create a working TCP implementation: a TCPConnection that contains a TCPSender and TCPReceiver. You’ll use this to talk to real servers around the world. 


# 3. Putting substrings in sequence

## Why am I doing this? 

TCP robustness against reordering and duplication comes from its ability to stitch arbitrary excerpts of the byte stream back into the original stream. Implementing this in a discrete testable module will make handling incoming segments much easier.


## TCP receiver 

the module that receives datagrams and turns them into a reliable byte stream to be read from the socket by the application

The TCP sender is dividing its byte stream up into short segments (substrings no more than about 1,460 bytes apiece) so that they each fit inside a datagram. But the network might reorder these datagrams, or drop them, or deliver them more than once. The receiver must reassemble the segments into the contiguous stream of bytes that they started out as.


In this lab you'll write the data structure that will be responsible for this reassembly: 
- a StreamReassembler
  It will receive substrings, consisting of a string of bytes, and the index of the first byte of that string within the larger stream. Each byte of the stream has its own unique index, starting from zero and counting upwards. The StreamReassembler will own a ByteStream for the output: as soon as the  reassembler knows the next byte of the stream, it will write it into the ByteStream. The owner can access and read from the ByteStream whenever it wants.


```C++
// Construct a `StreamReassembler` that will store up to `capacity` bytes.
StreamReassembler(const size_t capacity);
// Receive a substring and write any newly contiguous bytes into the stream,
// while staying within the memory limits of the `capacity`. Bytes that would
// exceed the capacity are silently discarded.
//
// `data`: the substring
// `index` indicates the index (place in sequence) of the first byte in `data`
// `eof`: the last byte of this substring will be the last byte in the entire stream
void push_substring(const string &data, const uint64_t index, const bool eof);
// Access the reassembled ByteStream (your code from Lab 0)
ByteStream &stream_out();
// The number of bytes in the substrings stored but not yet reassembled
size_t unassembled_bytes() const;
// Is the internal state empty (other than the output stream)?
bool empty() const;

```


## 3.1 What's the "capacity"?

Your push substring method will ignore any portion of the string that would cause the StreamReassembler to exceed its "capacity": a limit on memory usage, i.e. the maximum number of bytes it is ever allowed to store. 

This prevents the reassembler from using an unbounded amount of memory, no matter what the TCP sender decides to do. We've illustrated this in the picture below. The "capacity" is an upper bound on both:
1. The number of bytes in the reassembled ByteStream (shown in green below), and
2. The maximum number of bytes that can be used by "unassembled" substrings (shown
in red)