# CS144: Introduction to Computer Networking - Fall 2020

## Lab 1: stitching substrings into a byte stream

**Due:** Tuesday, October 6, 5 p.m. Stanford time (5% bonus if submitted by Tuesday, Sept. 29)  
**Lab sessions:** Wednesday, Sept. 23 & 30, 6–9 p.m. Stanford time

> **Note:** This is a two-part lab assignment. Labs 1 & 2 are both due on October 6, but there is a bonus for handing in Lab 1 a week early.

---

## 0. Collaboration Policy

The programming assignments must be your own work: You must write all the code you hand in for the programming assignments, except for the code that we give you as part of the assignment. Please do not copy-and-paste code from Stack Overflow, GitHub, or other sources. If you base your own code on examples you find on the Web or elsewhere, cite the URL in a comment in your submitted source code.

**Working with others:** You may not show your code to anyone else, look at anyone else's code, or look at solutions from previous years. You may discuss the assignments with other students, but do not copy anybody's code. If you discuss an assignment with another student, please name them in a comment in your submitted source code. Please refer to the course administrative handout for more details, and ask on Piazza if anything is unclear.

**Piazza:** Please feel free to ask questions on Piazza, but please don't post any source code.

---

## 1. Overview

In Lab 0, you used an Internet stream socket to fetch information from a website and send an email message, using Linux's built-in implementation of the Transmission Control Protocol (TCP). This TCP implementation managed to produce a pair of reliable in-order byte streams (one from you to the server, and one in the opposite direction), even though the underlying network only delivers "best-effort" datagrams. By this we mean: short packets of data that can be lost, reordered, altered, or duplicated. You also implemented the byte-stream abstraction yourself, in memory within one computer. Over the next four weeks, you'll implement TCP, to provide the byte-stream abstraction between a pair of computers separated by an unreliable datagram network.

> **Why am I doing this?** Providing a service or an abstraction on top of a different less-reliable service accounts for many of the interesting problems in networking. Over the last 40 years, researchers and practitioners have figured out how to convey all kinds of things—messaging and e-mail, hyperlinked documents, search engines, sound and video, virtual worlds, collaborative file sharing, digital currencies—over the Internet. TCP's own role, providing a pair of reliable byte streams using unreliable datagrams, is one of the classic examples of this. A reasonable view has it that TCP implementations count as the most widely used nontrivial computer programs on the planet.

### Architecture Overview


![image](../../images/labs/Figure%201-The%20arrangement%20of%20modules%20and%20data.png)

*Figure 1: The arrangement of modules and dataflow in your TCP implementation. The ByteStream was Lab 0. The job of TCP is to convey two ByteStreams (one in each direction) over an unreliable datagram network, so that bytes written to the socket on one side of the connection emerge as bytes that can be read at the peer, and vice versa. Lab 1 is the StreamReassembler, and in Labs 2, 3, and 4 you'll implement the TCPReceiver, TCPSender, and then the TCPConnection to tie it all together.*

The lab assignments will ask you to build up a TCP implementation in a modular way. Remember the ByteStream you just implemented in Lab 0? In the next four labs, you'll end up convey two of them across the network: an "outbound" ByteStream, for data that a local application writes to a socket and that your TCP will send to the peer, and an "inbound" ByteStream for data coming from the peer that will be read by a local application. Figure 1 shows how the pieces fit together.

1. **In Lab 1**, you'll implement a `stream reassembler` — a module that stitches small pieces of the byte stream (known as substrings, or segments) back into a contiguous stream of bytes in the correct sequence.
2. **In Lab 2**, you'll implement the part of TCP that handles the inbound byte-stream: the `TCPReceiver`. This involves thinking about how TCP will represent each byte's place in the stream—known as a "`sequence number`." 
   The TCPReceiver is responsible for telling the sender 
   (a) how much of the inbound byte stream it's been able to assemble successfully (this is called "`acknowledgment`") and 
   (b) how many more bytes the sender is allowed to send right now ("`flow control`").
3. **In Lab 3**, you'll implement the part of TCP that handles the outbound byte-stream: the `TCPSender`. How should the sender react when it suspects that a segment it transmitted was lost along the way and never made it to the receiver? When should it try again and re-transmit a lost segment?
4. **In Lab 4**, you'll combine your work from the previous to labs to create a working TCP implementation: a `TCPConnection` that contains a TCPSender and TCPReceiver. You'll use this to talk to real servers around the world.

---

## 2. Getting started

Your implementation of TCP will use the same Sponge library that you used in Lab 0, with additional classes and tests. To get started:

1. Make sure you have committed all your solutions to Lab 0. Please don't modify any files outside the top level of the `libsponge` directory, or `webget.cc`. You may have trouble merging the Lab 1 starter code otherwise.
2. While inside the repository for the lab assignments, run `git fetch` to retrieve the most recent version of the lab assignments.
3. Download the starter code for Lab 1 by running `git merge origin/lab1-startercode`.
4. Within your build directory, compile the source code: `make` (you can run, e.g., `make -j4` to use four processors when compiling).
5. Outside the build directory, open and start editing the `writeups/lab1.md` file. This is the template for your lab writeup and will be included in your submission.

---

## 3. Putting substrings in sequence

In this and the next lab, you will implement a TCP receiver: 
the module that receives `datagrams` and turns them into a reliable byte stream to be read from the socket by the application—just as your `webget` program read the byte stream from the webserver in Lab 0.

The TCP sender is dividing its byte stream up into short `segments` (substrings no more than about 1,460 bytes a piece) so that they each fit inside a datagram. But the network might reorder these datagrams, or drop them, or deliver them more than once. The receiver must reassemble the segments into the contiguous stream of bytes that they started out as.

In this lab you'll write the data structure that will be responsible for this reassembly: a `StreamReassembler`. It will receive substrings, consisting of a string of bytes, and the index of the first byte of that string within the larger stream. Each byte of the stream has its own unique index, starting from zero and counting upwards. The `StreamReassembler` will own a `ByteStream` for the output: as soon as the reassembler knows the next byte of the stream, it will write it into the `ByteStream`. The owner can access and read from the `ByteStream` whenever it wants.

### Interface

```cpp
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

> **Why am I doing this?** TCP robustness against reordering and duplication comes from its ability to stitch arbitrary excerpts of the byte stream back into the original stream. Implementing this in a discrete testable module will make handling incoming segments much easier.

The full (public) interface of the reassembler is described by the `StreamReassembler` class in the `stream_reassembler.hh` header. Your task is to implement this class. You may add any private members and member functions you desire to the `StreamReassembler` class, but you cannot change its public interface.

### 3.1 What's the "capacity"?

Your `push_substring` method will ignore any portion of the string that would cause the `StreamReassembler` to exceed its "capacity": a limit on memory usage, i.e. the maximum number of bytes it is ever allowed to store. This prevents the reassembler from using an unbounded amount of memory, no matter what the TCP sender decides to do. We've illustrated this in the picture below. The "capacity" is an upper bound on both:

1. The number of bytes in the reassembled `ByteStream` (shown in green below), and
2. The maximum number of bytes that can be used by "unassembled" substrings (shown in red)

You may find this picture useful as you implement the StreamReassembler and work through
the tests|it's not always natural what the \right" behavior is.

### 3.2 FAQs

- **What is the index of the first byte in the whole stream?** Zero.
- **How efficient should my implementation be?** Please don't take this as a challenge to build a grossly space- or time-inefficient data structure—this data structure will be the foundation of your TCP implementation. A ballpark expectation would be that each of the new Lab 1 tests can complete in less than half a second.
- **How should inconsistent substrings be handled?** You may assume that they don't exist. That is, you can assume that there is a unique underlying byte-stream, and all substrings are (accurate) slices of it.
- **What may I use?** You may use any part of the standard library you find helpful. In particular, we expect you to use at least one data structure.
- **When should bytes be written to the stream?** As soon as possible. The only situation in which a byte should not be in the stream is that when there is a byte before it that has not been "pushed" yet.
- **May substrings provided to the `push_substring()` function overlap?** Yes.
- **Will I need to add private members to the `StreamReassembler`?** Yes. Substrings may arrive in any order, so your data structure will have to "remember" substrings until they're ready to be put into the stream—that is, until all indices before them have been written.
- **Is it okay for our re-assembly data structure to store overlapping substrings?** No. It is possible to implement an "interface-correct" reassembler that stores overlapping substrings. But allowing the re-assembler to do this undermines the notion of "capacity" as a memory limit. We'll consider the storage of overlapping substrings to be a style violation when grading.
- **More FAQs:** For more, please see https://cs144.github.io/lab_faq.html.

---

## 4. Development and debugging advice

1. You can test your code (after compiling it) with `make check_lab1`.
2. Please re-read the section on "using Git" in the Lab 0 document, and remember to keep the code in the Git repository it was distributed in on the master branch. Make small commits, using good commit messages that identify what changed and why.
3. Please work to make your code readable to the CA who will be grading it for style. Use reasonable and clear naming conventions for variables. Use comments to explain complex or subtle pieces of code. Use "defensive programming"—explicitly check preconditions of functions or invariants, and throw an exception if anything is ever wrong. Use modularity in your design—identify common abstractions and behaviors and factor them out when possible. Blocks of repeated code and enormous functions will make it hard to follow your code.
4. Please also keep to the "Modern C++" style described in the Lab 0 document. The cppreference website (https://en.cppreference.com) is a great resource, although you won't need any sophisticated features of C++ to do these labs. (You may sometimes need to use the `move()` function to pass an object that can't be copied.)
5. If you get a segmentation fault, something is really wrong! We would like you to be writing in a style where you use safe programming practices to make segfaults extremely unusual (no `malloc()`, no `new`, no pointers, safety checks that throw exceptions where you are uncertain, etc.). That said, to debug you can configure your build directory with `cmake .. -DCMAKE_BUILD_TYPE=RelASan` to enable the compiler's "sanitizers" to detect memory errors and undefined behavior and give you a nice diagnostic about when they occur. You can also use the `valgrind` tool. You can also configure with `cmake .. -DCMAKE_BUILD_TYPE=Debug` and use the GNU debugger (`gdb`). But please remember that these options (especially the sanitizers) will slow down compilation and execution—you don't want to accidentally leave them in place!
6. You can reset the build system with `make clean` and `cmake .. -DCMAKE_BUILD_TYPE=Release`. Or if you get your builds really stuck and aren't sure how to fix them, you can erase your build directory (`rm -rf build` —please be careful not to make a typo as this will erase whatever you tell it), make a new build directory, and `cmake ..` again.

---

## 5. Submit

1. In your submission, please only make changes to the `.hh` and `.cc` files in the top level of `libsponge`. Within these files, please feel free to add private members as necessary, but please don't change the public interface of any of the classes.
2. Before handing in any assignment, please run these in order:
   - (a) `make format` (to normalize the coding style)
   - (b) `make` (to make sure the code compiles)
   - (c) `make check_lab1` (to make sure the automated tests pass)
3. Write a report in `writeups/lab1.md`. This file should be a roughly 20-to-50-line document with no more than 80 characters per line to make it easier to read. The report should contain the following sections:
   - (a) **Program Structure and Design.** Describe the high-level structure and design choices embodied in your code. You do not need to discuss in detail what you inherited from the starter code. Use this as an opportunity to highlight important design aspects and provide greater detail on those areas for your grading TA to understand. You are strongly encouraged to make this writeup as readable as possible by using subheadings and outlines. Please do not simply translate your program into an paragraph of English.
   - (b) **Implementation Challenges.** Describe the parts of code that you found most troublesome and explain why. Reflect on how you overcame those challenges and what helped you finally understand the concept that was giving you trouble. How did you attempt to ensure that your code maintained your assumptions, invariants, and preconditions, and in what ways did you find this easy or difficult? How did you debug and test your code?
   - (c) **Remaining Bugs.** Point out and explain as best you can any bugs (or unhandled edge cases) that remain in the code.
4. In your writeup, please also fill in the number of hours the assignment took you and any other comments.
5. When ready to submit, please follow the instructions at https://cs144.github.io/submit. Please make sure you have committed everything you intend before submitting.
6. Please let the course staff know ASAP of any problems at the Wednesday-evening lab session, or by posting a question on Piazza. Good luck!


# Solution

### Understand The Problem Visually
Suppose original stream is:

```
Index:
0 1 2 3 4 5 6 7 8 9

Data:
H E L L O W O R L D
```

But network delivers:
```
push_substring("WORLD", 5)
push_substring("HEL", 0)
push_substring("LO", 3)
```
Your reassembler must eventually output:`HELLOWORLD`  in correct order.


### Big Goal
Write bytes into _output ONLY when they become contiguous and in-order.

### The Most Important Concept
“First Unassembled Byte” : This is the central idea of the whole lab.



# Test
## Test check_lab1

```bash
cd ~/computer_network/sponge/build
rm -rf *                          # Clean build
# Do not use -DCMAKE_BUILD_TYPE=Release or sanitizers (RelASan) for basic breakpoint debugging.
cmake -DCMAKE_BUILD_TYPE=Debug .. # Enable debug info
make -j8


cs144@cs144vm:~/computer_network/sponge/build$ make check_lab1
[100%] Testing the stream reassembler...
Test project /home/cs144/computer_network/sponge/build
      Start 18: t_strm_reassem_single
 1/16 Test #18: t_strm_reassem_single ............***Failed    0.01 sec
Test Failure on expectation:
        Expectation: net bytes assembled = 1

Failure message:
        The reassembler was expected to have `1` total bytes assembled, but there were `0`

List of steps that executed successfully:
        Initialized (capacity = 65000)
        Action:      substring submitted with data "a", index `0`, eof `0`

Exception: The reassembler was expected to have `1` total bytes assembled, but there were `0`

      Start 19: t_strm_reassem_seq
 2/16 Test #19: t_strm_reassem_seq ...............***Failed    0.01 sec
Test Failure on expectation:
        Expectation: net bytes assembled = 4

Failure message:
        The reassembler was expected to have `4` total bytes assembled, but there were `0`

List of steps that executed successfully:
        Initialized (capacity = 65000)
        Action:      substring submitted with data "abcd", index `0`, eof `0`

Exception: The reassembler was expected to have `4` total bytes assembled, but there were `0`

      Start 20: t_strm_reassem_dup
 3/16 Test #20: t_strm_reassem_dup ...............***Failed    0.01 sec
Test Failure on expectation:
        Expectation: net bytes assembled = 4

Failure message:
        The reassembler was expected to have `4` total bytes assembled, but there were `0`

List of steps that executed successfully:
        Initialized (capacity = 65000)
        Action:      substring submitted with data "abcd", index `0`, eof `0`

Exception: The reassembler was expected to have `4` total bytes assembled, but there were `0`

      Start 21: t_strm_reassem_holes
 4/16 Test #21: t_strm_reassem_holes .............***Failed    0.01 sec
Test Failure on expectation:
        Expectation: net bytes assembled = 2

Failure message:
        The reassembler was expected to have `2` total bytes assembled, but there were `1`

List of steps that executed successfully:
        Initialized (capacity = 65000)
        Action:      substring submitted with data "b", index `1`, eof `0`
        Action:      substring submitted with data "a", index `0`, eof `0`

Exception: The reassembler was expected to have `2` total bytes assembled, but there were `1`

      Start 22: t_strm_reassem_many
 5/16 Test #22: t_strm_reassem_many ..............***Failed    0.05 sec
Exception: test 1 - number of bytes RX is incorrect

      Start 23: t_strm_reassem_overlapping
 6/16 Test #23: t_strm_reassem_overlapping .......***Failed    0.01 sec
Test Failure on expectation:
        Expectation: net bytes assembled = 2

Failure message:
        The reassembler was expected to have `2` total bytes assembled, but there were `0`

List of steps that executed successfully:
        Initialized (capacity = 1000)
        Action:      substring submitted with data "a", index `0`, eof `0`
        Action:      substring submitted with data "ab", index `0`, eof `0`

Exception: The reassembler was expected to have `2` total bytes assembled, but there were `0`

      Start 24: t_strm_reassem_win
 7/16 Test #24: t_strm_reassem_win ...............***Failed    0.05 sec
Exception: test 2 - number of RX bytes is incorrect

      Start 25: t_strm_reassem_cap
 8/16 Test #25: t_strm_reassem_cap ...............***Failed    0.01 sec
Test Failure on expectation:
        Expectation: net bytes assembled = 2

Failure message:
        The reassembler was expected to have `2` total bytes assembled, but there were `0`

List of steps that executed successfully:
        Initialized (capacity = 2)
        Action:      substring submitted with data "ab", index `0`, eof `0`

Exception: The reassembler was expected to have `2` total bytes assembled, but there were `0`

      Start 26: t_byte_stream_construction
 9/16 Test #26: t_byte_stream_construction .......   Passed    0.01 sec
      Start 27: t_byte_stream_one_write
10/16 Test #27: t_byte_stream_one_write ..........   Passed    0.01 sec
      Start 28: t_byte_stream_two_writes
11/16 Test #28: t_byte_stream_two_writes .........   Passed    0.01 sec
      Start 29: t_byte_stream_capacity
12/16 Test #29: t_byte_stream_capacity ...........   Passed    0.46 sec
      Start 30: t_byte_stream_many_writes
13/16 Test #30: t_byte_stream_many_writes ........   Passed    0.02 sec
      Start 48: t_address_dt
14/16 Test #48: t_address_dt .....................   Passed    0.08 sec
      Start 49: t_parser_dt
15/16 Test #49: t_parser_dt ......................   Passed    0.01 sec
      Start 50: t_socket_dt
16/16 Test #50: t_socket_dt ......................   Passed    0.01 sec

50% tests passed, 8 tests failed out of 16

Total Test time (real) =   0.79 sec

The following tests FAILED:
         18 - t_strm_reassem_single (Failed)
         19 - t_strm_reassem_seq (Failed)
         20 - t_strm_reassem_dup (Failed)
         21 - t_strm_reassem_holes (Failed)
         22 - t_strm_reassem_many (Failed)
         23 - t_strm_reassem_overlapping (Failed)
         24 - t_strm_reassem_win (Failed)
         25 - t_strm_reassem_cap (Failed)
Errors while running CTest
make[3]: *** [CMakeFiles/check_lab1.dir/build.make:71: CMakeFiles/check_lab1] Error 8
make[2]: *** [CMakeFiles/Makefile2:7440: CMakeFiles/check_lab1.dir/all] Error 2
make[1]: *** [CMakeFiles/Makefile2:7447: CMakeFiles/check_lab1.dir/rule] Error 2
make: *** [Makefile:2773: check_lab1] Error 2
```



## Debug Process

### Step 1: Build with Debug Symbols

```bash
cd ~/computer_network/sponge/build
rm -rf *                          # Clean build
# Do not use -DCMAKE_BUILD_TYPE=Release or sanitizers (RelASan) for basic breakpoint debugging.
cmake -DCMAKE_BUILD_TYPE=Debug .. # Enable debug info
make -j8

# test
make check_lab1
```

### Step 2: Locate the Test Executable

```bash
# The failing tests (e.g., t_strm_reassem_single) are individual binaries. Find them in your build directory:
ls build/tests/
# You'll see: t_strm_reassem_single, t_strm_reassem_seq, etc.
# For example: 
# ./build/tests/t_strm_reassem_single
```


t_strm_reassem_single is usually generated into the build tree, but not necessarily under /build/tests. In CS144/sponge, many test executables are placed directly in: build/tests/ depending on the CMake configuration.
But even if you don't immediately see it, CTest already knows exactly where the executable is.
The easiest way to debug is:

### Find The Actual Executable
```bash
# This lists all registered tests.
ctest -N
Test project /home/cs144/computer_network/sponge/build
  Test   #1: t_wrapping_ints_cmp
  Test   #2: t_wrapping_ints_unwrap
  Test   #3: t_wrapping_ints_wrap
  Test   #4: t_wrapping_ints_roundtrip
  Test   #5: t_recv_connect
  Test   #6: t_recv_transmit
  Test   #7: t_recv_window
  Test   #8: t_recv_reorder
  Test   #9: t_recv_close
  Test  #10: t_recv_special
  Test  #11: t_send_connect
  Test  #12: t_send_transmit
  Test  #13: t_send_retx
  Test  #14: t_send_window
  Test  #15: t_send_ack
  Test  #16: t_send_close
  Test  #17: t_send_extra
  Test  #18: t_strm_reassem_single
  Test  #19: t_strm_reassem_seq
  Test  #20: t_strm_reassem_dup
  Test  #21: t_strm_reassem_holes
  Test  #22: t_strm_reassem_many
  Test  #23: t_strm_reassem_overlapping
  Test  #24: t_strm_reassem_win
  Test  #25: t_strm_reassem_cap
  Test  #26: t_byte_stream_construction
  Test  #27: t_byte_stream_one_write
  Test  #28: t_byte_stream_two_writes
  Test  #29: t_byte_stream_capacity
  Test  #30: t_byte_stream_many_writes
  Test  #31: t_webget
  ....


# THAT is the actual executable path.
ctest -V -R t_strm_reassem_single

UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
Test project /home/cs144/computer_network/sponge/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 18
    Start 18: t_strm_reassem_single

18: Test command: /home/cs144/computer_network/sponge/build/tests/fsm_stream_reassembler_single
18: Working Directory: /home/cs144/computer_network/sponge/build
18: Test timeout computed to be: 10000000
18: Test Failure on expectation:
18:     Expectation: net bytes assembled = 1
18: 
18: Failure message:
18:     The reassembler was expected to have `1` total bytes assembled, but there were `0`
18: 
18: List of steps that executed successfully:
18:     Initialized (capacity = 65000)
18:     Action:      substring submitted with data "a", index `0`, eof `0`
18: 
18: Exception: The reassembler was expected to have `1` total bytes assembled, but there were `0`
1/1 Test #18: t_strm_reassem_single ............***Failed    0.01 sec

0% tests passed, 1 tests failed out of 1

Total Test time (real) =   0.01 sec

The following tests FAILED:
         18 - t_strm_reassem_single (Failed)
Errors while running CTest
Output from these tests are in: /home/cs144/computer_network/sponge/build/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.

```
### Step 3: Configure VS Code for Debugging

1. Create .vscode/launch.json

use actual executable path  `fsm_stream_reassembler_single` instead of `t_strm_reassem_single`
```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug Labl1-StreamReassembler",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/sponge/build/tests/fsm_stream_reassembler_single",
      "args": [],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}/sponge/build",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ],
      "preLaunchTask": "build cs144-sponge"
    }
  ]
}
```

#### C. (Optional) Add a Build Task

2.  Create .vscode/tasks.json to auto-build before debugging:
```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "build cs144-sponge",
      "type": "shell",
      "command": "make -j8",
      "group": "build",
      "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "shared"
      },
      "options": {
        "cwd": "${workspaceFolder}/sponge/build"
      }
    }
  ]
}
```


### Step 4: Set Breakpoints in Your Code

test executable: ``computer_network/sponge/build/tests/fsm_stream_reassembler_single``


#### A. set a breakpoint in Unit Test Source Code
You should open: `sponge/tests/`

tests/fsm_stream_reassembler_single.cc
tests/fsm_stream_reassembler_seq.cc


#### B. set a breakpoint in Implementation Source Code
You should open: `computer_network/sponge/libsponge/`

stream_reassembler.cc

### Step 5: Start Debugging


## Fixed

after 1st fix:
```bash
cs144@cs144vm:~/computer_network/sponge/build$ make check_lab1
[100%] Testing the stream reassembler...
Test project /home/cs144/computer_network/sponge/build
      Start 18: t_strm_reassem_single
 1/16 Test #18: t_strm_reassem_single ............***Failed    0.01 sec
Test Failure on expectation:
        Expectation: at EOF

Failure message:
        The reassembler was expected to be at EOF, but was not

List of steps that executed successfully:
        Initialized (capacity = 65000)
        Action:      substring submitted with data "", index `0`, eof `1`
        Expectation: net bytes assembled = 0
        Expectation: stream_out().buffer_size() returned 0, and stream_out().read(0) returned the string ""

Exception: The reassembler was expected to be at EOF, but was not

      Start 19: t_strm_reassem_seq
 2/16 Test #19: t_strm_reassem_seq ...............   Passed    0.01 sec
      Start 20: t_strm_reassem_dup
 3/16 Test #20: t_strm_reassem_dup ...............   Passed    0.01 sec
      Start 21: t_strm_reassem_holes
 4/16 Test #21: t_strm_reassem_holes .............***Failed    0.01 sec
Test Failure on expectation:
        Expectation: at EOF

Failure message:
        The reassembler was expected to be at EOF, but was not

List of steps that executed successfully:
        Initialized (capacity = 65000)
        Action:      substring submitted with data "b", index `1`, eof `0`
        Expectation: net bytes assembled = 0
        Expectation: stream_out().buffer_size() returned 0, and stream_out().read(0) returned the string ""
        Expectation: not at EOF
        Action:      substring submitted with data "d", index `3`, eof `0`
        Expectation: net bytes assembled = 0
        Expectation: stream_out().buffer_size() returned 0, and stream_out().read(0) returned the string ""
        Expectation: not at EOF
        Action:      substring submitted with data "a", index `0`, eof `0`
        Expectation: net bytes assembled = 2
        Expectation: stream_out().buffer_size() returned 2, and stream_out().read(2) returned the string "ab"
        Expectation: not at EOF
        Action:      substring submitted with data "c", index `2`, eof `0`
        Expectation: net bytes assembled = 4
        Expectation: stream_out().buffer_size() returned 2, and stream_out().read(2) returned the string "cd"
        Expectation: not at EOF
        Action:      substring submitted with data "", index `4`, eof `1`
        Expectation: net bytes assembled = 4
        Expectation: stream_out().buffer_size() returned 0, and stream_out().read(0) returned the string ""

Exception: The reassembler was expected to be at EOF, but was not

      Start 22: t_strm_reassem_many
 5/16 Test #22: t_strm_reassem_many ..............   Passed    0.98 sec
      Start 23: t_strm_reassem_overlapping
 6/16 Test #23: t_strm_reassem_overlapping .......   Passed    0.01 sec
      Start 24: t_strm_reassem_win
 7/16 Test #24: t_strm_reassem_win ...............   Passed    1.07 sec
      Start 25: t_strm_reassem_cap
 8/16 Test #25: t_strm_reassem_cap ...............   Passed    0.08 sec
      Start 26: t_byte_stream_construction
 9/16 Test #26: t_byte_stream_construction .......   Passed    0.01 sec
      Start 27: t_byte_stream_one_write
10/16 Test #27: t_byte_stream_one_write ..........   Passed    0.00 sec
      Start 28: t_byte_stream_two_writes
11/16 Test #28: t_byte_stream_two_writes .........   Passed    0.00 sec
      Start 29: t_byte_stream_capacity
12/16 Test #29: t_byte_stream_capacity ...........   Passed    0.52 sec
      Start 30: t_byte_stream_many_writes
13/16 Test #30: t_byte_stream_many_writes ........   Passed    0.01 sec
      Start 48: t_address_dt
14/16 Test #48: t_address_dt .....................   Passed    0.07 sec
      Start 49: t_parser_dt
15/16 Test #49: t_parser_dt ......................   Passed    0.01 sec
      Start 50: t_socket_dt
16/16 Test #50: t_socket_dt ......................   Passed    0.01 sec

88% tests passed, 2 tests failed out of 16

Total Test time (real) =   2.83 sec

The following tests FAILED:
         18 - t_strm_reassem_single (Failed)
         21 - t_strm_reassem_holes (Failed)
Errors while running CTest
make[3]: *** [CMakeFiles/check_lab1.dir/build.make:71: CMakeFiles/check_lab1] Error 8
make[2]: *** [CMakeFiles/Makefile2:7440: CMakeFiles/check_lab1.dir/all] Error 2
make[1]: *** [CMakeFiles/Makefile2:7447: CMakeFiles/check_lab1.dir/rule] Error 2
make: *** [Makefile:2773: check_lab1] Error 2



cs144@cs144vm:~/computer_network/sponge/build$ ctest -V -R t_strm_reassem_single
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/cs144/computer_network/sponge/build/DartConfiguration.tcl
Test project /home/cs144/computer_network/sponge/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 18
    Start 18: t_strm_reassem_single

18: Test command: /home/cs144/computer_network/sponge/build/tests/fsm_stream_reassembler_single
18: Working Directory: /home/cs144/computer_network/sponge/build
18: Test timeout computed to be: 10000000
18: Test Failure on expectation:
18:     Expectation: at EOF
18: 
18: Failure message:
18:     The reassembler was expected to be at EOF, but was not
18: 
18: List of steps that executed successfully:
18:     Initialized (capacity = 65000)
18:     Action:      substring submitted with data "", index `0`, eof `1`
18:     Expectation: net bytes assembled = 0
18:     Expectation: stream_out().buffer_size() returned 0, and stream_out().read(0) returned the string ""
18: 
18: Exception: The reassembler was expected to be at EOF, but was not
1/1 Test #18: t_strm_reassem_single ............***Failed    0.01 sec

0% tests passed, 1 tests failed out of 1

Total Test time (real) =   0.01 sec

The following tests FAILED:
         18 - t_strm_reassem_single (Failed)
Errors while running CTest
Output from these tests are in: /home/cs144/computer_network/sponge/build/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.



```
