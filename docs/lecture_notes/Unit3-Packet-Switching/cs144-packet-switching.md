# CS144: Packet Switching — A Developer's Guide

**CS144 · Stanford University · Unit 3**

Every time you load a web page, stream a video, or make a video call, billions of tiny packets are flying across the Internet at the speed of light. This tutorial — drawn from Stanford's CS144 — explains exactly how that works: what packet switching is, why the Internet chose it, how routers decide where to send packets, and how engineers guarantee that your real-time video call won't stutter. No networking background required.

---

## Section 01 — A Brief History of Networks

The Internet may feel like a recent invention, but the fundamental ideas behind it — encoding information, routing it across a network, dealing with errors, and controlling the flow of data — are centuries old.

### Ancient Long-Distance Communication

The first recorded long-distance communications date from around 1,000 BC, mostly for military purposes. Fire beacons could signal danger at the speed of light, but carried almost no information — just "enemy approaching" or "attack." Carrier pigeons and horse relays could carry more information, but traveled slowly and could be intercepted. Around 2,000 years ago, optical methods such as flags and heliographs emerged, encoding digital information (letters, words, numbers) at the speed of light.

### Claude Chappe and the Birth of Protocols

The biggest leap in optical communications happened during the French Revolution. In 1793, Claude Chappe built a semaphore telegraph network: towers topped with a horizontal beam and two smaller arms, whose positions encoded symbols. Operators could send messages across France in under 30 minutes — an extraordinary achievement.

To make the network function reliably, Chappe's engineers invented five concepts that are *still used in every computer network today*:

| Concept | What it does | Modern equivalent |
|---|---|---|
| Codes | Symbols for characters and control signals (START, WAIT, ERROR) | Packet headers, protocol opcodes |
| Flow Control | Tell the sender to slow down when the receiver can't keep up | TCP receive window |
| Synchronization | Mark where one symbol ends and the next begins | Framing bytes, clock recovery |
| Error Correction & Retransmission | Tell the sender when a symbol was garbled; ask for a resend | TCP ACKs, checksums |
| Encryption | Prevent eavesdropping | TLS/HTTPS |

### The Telephone and Circuit Switching

Alexander Graham Bell transmitted the first voice call in 1876. Within 10 years, over 150,000 people owned telephones; by 1915 the first transcontinental call crossed the USA. The telephone network introduced **circuit switching** — dedicating a physical path (a circuit) from caller to receiver for the entire duration of a call. We'll contrast this with packet switching in the next section.

### The Road to the Internet

The chain of events leading to the Internet began in 1960. J.C.R. Licklider at MIT wrote about an "Intergalactic Network" — everyone on the globe connected, able to access programs and data anywhere. He became the first head of DARPA's computer research program in 1962, seeding the ideas that would become the ARPANET.

In 1965, Larry Roberts connected two computers across a telephone line — the first wide-area computer network. In 1969, the first four ARPANET nodes went live at UCLA, SRI, UCSB, and the University of Utah. In 1974, Vint Cerf (then at Stanford) and Bob Kahn described TCP in their landmark paper, "A Protocol for Packet Network Intercommunication." They are considered the fathers of the Internet.

By 1990, Tim Berners-Lee at CERN invented the World Wide Web. In 1993 the Mosaic browser appeared, and within a year, over a million people were using the Web.

> **Key Takeaway:** Networking concepts like flow control, error correction, and protocol encoding predate computers by centuries. The Internet didn't invent these ideas — it built on 200 years of hard-won engineering insight from telegraph operators, telephone engineers, and early computer scientists.

---

## Section 02 — What Is Packet Switching — And Why Does the Internet Use It?

To understand packet switching, you first need to understand what it replaced.

### Circuit Switching: The Telephone Model

When you make a traditional phone call, the network creates a **dedicated circuit** — a reserved path of physical connections from your phone to the other. Think of it like booking an entire lane of a highway just for your car. The circuit is yours for the whole call, regardless of whether you're talking or sitting in silence.

In a digital telephone network, each voice call gets a dedicated 64 kb/s channel. Hundreds of thousands of such circuits share the high-speed "trunk lines" between cities — each one private and unshared. When you hang up, the circuit is torn down and all its reserved resources are freed.

**Circuit Switching:**
- Dedicated circuit reserved end-to-end
- Guaranteed bandwidth and delay
- Resources wasted when line is idle
- State maintained in every switch
- Setup phase required before data flows

**Packet Switching:**
- No dedicated circuit — links are shared
- Bandwidth used only when sending
- Efficient: many flows share one link
- Little persistent state in switches
- Send immediately — no setup needed

### Packet Switching: The Post Office Model

First described by Paul Baran in the early 1960s, **packet switching** works like the postal system. Instead of reserving a private lane, you take your message, chop it into smaller pieces called **packets**, and attach a header to each one — just like writing an address on an envelope.

Each packet travels independently through the network. At every **packet switch** (router) along the way, the switch reads the destination address in the header, looks it up in a local **forwarding table**, and sends the packet out the correct port toward its destination. This is called **hop-by-hop routing**.

```python
# Packet forwarding — conceptual pseudocode
def forward_packet(packet):
    dest = packet.header.destination_address
    next_hop = forwarding_table.lookup(dest)

    if next_hop is not None:
        output_queue[next_hop].enqueue(packet)
    else:
        drop(packet)   # No route found
```

*Every router in the Internet runs logic like this for every packet. The forwarding table is what makes routing decisions — it maps destination addresses to output ports.*

### Why Packet Switching Wins

Imagine 1,000 users each sending occasional short messages over the same network. In a circuit-switched network you'd need 1,000 dedicated circuits — even though most of them are idle most of the time. In a packet-switched network, all 1,000 users share the same links, and those links are only busy when someone is actually transmitting. This is called **statistical multiplexing**.

There's another benefit: breaking a large message into many small packets allows **parallel transmission** across multiple links simultaneously. Different packets can be in-flight on different hops at the same time, dramatically reducing end-to-end latency for large transfers.

> **Key Takeaway:** Packet switching replaced dedicated circuits with shared, opportunistic transmission. By adding addresses to data chunks (packets) and routing them hop-by-hop, the Internet achieves high efficiency, robustness, and low cost — at the price of variable, unpredictable delay.

---

## Section 03 — Propagation, Packetization, and End-to-End Delay

When a packet travels from A to B, time passes for several distinct reasons. Understanding each type of delay is essential for building real-time applications.

### Propagation Delay

**Propagation delay** is the time it takes a single bit to travel from one end of a link to the other. It depends only on the length of the cable and the speed at which signals travel through that medium — roughly the speed of light.

```
propagation_delay = link_length / propagation_speed

Example: 300 km fiber link at 2×10⁸ m/s → 1.5 ms
```

Crucially, propagation delay does *not* depend on the data rate of the link. A bit travels just as fast on a 1 kb/s link as on a 100 Gb/s link — the speed of light doesn't care how wide the pipe is.

### Packetization Delay

**Packetization delay** (also called transmission delay) is the time from when the first bit of a packet is placed onto the link until the last bit is sent. Think of it like the time it takes to pour a glass of water — it depends on the size of the glass (packet size) and how fast you pour (data rate).

```
packetization_delay = packet_size_in_bits / link_data_rate

Example: 12,000-bit packet on 100 Mb/s link → 0.12 ms
```

Packetization delay does *not* depend on the length of the link.

### Store-and-Forward Switching

Internet routers are **store-and-forward** switches — they wait until the *entire* packet has arrived before looking up the destination and forwarding it. This means the packetization delay is incurred at every hop.

### End-to-End Delay

The total delay for a packet traversing *n* links is the sum of propagation and packetization delays across every link, plus queueing delay:

```
end_to_end_delay = Σᵢ (packet_size / rᵢ) + Σᵢ (lᵢ / c) + Σᵢ queueing_delay_at_switch_i
```

Queueing delay is the time a packet spends waiting in a router's buffer while other packets ahead of it are being transmitted. It is the **only unpredictable component** — it depends on what everyone else on the Internet is doing right now.

### Measuring It: The Ping Tool

The `ping` command measures round-trip time (RTT). Measurements from Stanford to Princeton show RTTs clustering around 100–120 ms with about 50 ms of variation. Measurements to Tsinghua University in China show RTTs from 320 ms to over 500 ms — a 200 ms spread, almost entirely due to variable queueing delay.

> **Key Takeaway:** End-to-end delay has two deterministic parts (propagation, packetization) and one unpredictable part (queueing). The queueing delay — caused by packets waiting in router buffers while other users' traffic is served first — is what makes network delay variable and hard to guarantee.

---

## Section 04 — Playback Buffers: How Streaming Video Handles Variable Delay

Most applications — loading a web page, sending email — don't mind if packets arrive with slightly varying delays. But **real-time applications** like streaming video and voice calls must play back content at a steady, constant rate. If a video is encoded at 1 Mb/s, the player must display frames at exactly 30 fps. If a packet arrives late, there's no data to show and the screen freezes.

### The Problem: Jitter

Imagine a YouTube server sending video at 1 Mb/s. The client receives the data, but because of variable queueing delays in routers along the path, the bytes don't arrive at a perfectly smooth rate — they arrive in bursts and gaps.

### The Solution: Buffer Before Playback

The fix is the **playback buffer**. The client deliberately waits and accumulates a reserve of data before starting playback. Think of it like filling a bathtub before getting in — you wait until there's enough water, then you can enjoy a steady supply even if the tap temporarily slows.

You've seen this in action on YouTube: that grey bar that fills up ahead of the red playback indicator. If a burst of queueing delay hits some packets on the way, the player can draw from the buffer rather than freezing.

### The Tradeoff: Buffer Size vs. Startup Latency

A bigger playback buffer makes freezing less likely, but it increases startup latency. If the buffer runs dry — called a **rebuffering event** — playback freezes while the client waits for more data to arrive. You can fix rebuffering by streaming at a lower quality level, or by downloading the video ahead of time.

> **Key Takeaway:** Streaming applications cannot eliminate variable network delay — so instead they absorb it using a playback buffer. By pre-buffering ahead of the playback point, the player shields the user from temporary delivery gaps, trading startup latency for smooth, uninterrupted playback.

---

## Section 05 — A Simple Deterministic Queue Model

To reason about packet delay, we need a mathematical model of a router's queue.

### A(t) and D(t)

Define **A(t)** as the cumulative number of bytes that have *arrived* at a queue up until time t. Define **D(t)** as the cumulative number of bytes that have *departed* up until time t. Both are non-decreasing, and A(t) ≥ D(t) always.

```
Queue occupancy:    Q(t) = A(t) − D(t)    [bytes currently in the buffer]
Queueing delay:     d(t) = horizontal distance between A(t) and D(t) at time t
```

[Diagram: A graph showing cumulative bytes on the y-axis and time on the x-axis. The A(t) curve climbs steeply during a burst then levels off; the D(t) curve climbs more gently at the constant service rate R. The vertical gap between the two curves at any time t is the queue occupancy Q(t). The horizontal gap between them at a given height is the queueing delay d(t) experienced by a byte arriving at that time.]

### Worked Example

```python
# At the start of every second, 100 bits arrive at 1000 bits/s.
# Service rate = 500 bits/s.

arrival_rate   = 1000  # bits/s during the burst
burst_size     = 100   # bits per train
service_rate   = 500   # bits/s
burst_duration = burst_size / arrival_rate  # = 0.1 s

# During 0 → 0.1s: bits arrive at 1000 b/s, depart at 500 b/s
# Queue builds at 500 b/s, reaching peak of 50 bits at t=0.1s

# During 0.1s → 0.2s: no more arrivals, queue drains at 500 b/s
# Queue empty at t=0.2s

# During 0.2s → 1.0s: queue stays empty (0.8s idle)

avg_occupancy_first_0_2s = 0.5 * (0.1 * 500)  # = 25 bits
time_avg_occupancy = (0.2 * 25) + (0.8 * 0)   # = 5 bits
```

*The queue fills for 0.1 s, drains for 0.1 s, then sits idle for 0.8 s. The time-weighted average occupancy is just 5 bits — much lower than the 50-bit peak.*

### Why Break Messages Into Packets?

```
One big packet:      t = M/rᵢ + Σ lᵢ/c       (sequential transmission per hop)
Many small packets:  t = p/rᵢ + Σ lᵢ/c + (M/p − 1) × p/rₘᵢₙ   (pipelined)
```

The pipelined version finishes much sooner because intermediate links are kept busy with subsequent packets while early packets move ahead.

### Statistical Multiplexing

When many flows share a single output link, the link doesn't need to run at the sum of all their rates. Because different users are active at different times, a buffer absorbs the brief periods when combined traffic exceeds the link rate. This is **statistical multiplexing gain**. The tradeoff: a finite buffer means packets may be dropped if too many users burst at once.

> **Key Takeaway:** The A(t)/D(t) model gives us a concrete way to reason about queue occupancy and delay. Breaking messages into packets enables pipelining. Statistical multiplexing lets one shared link efficiently serve many users, because their peak demands rarely coincide.

---

## Section 06 — Useful Queue Properties

When arrival processes are random, queueing theory gives us powerful results.

### Property 1: Burstiness Increases Delay

Compare two scenarios with the same average arrival rate: (a) packets arrive one at a time, evenly spaced, and (b) packets arrive in bursts. Scenario (b) has higher average queueing delay. When a burst arrives, many packets pile up simultaneously; later packets in the burst must wait for all the earlier ones to depart. The more bursty the traffic, the longer packets wait on average.

### Property 2: Determinism Minimizes Delay

Given a fixed average arrival rate, the arrival pattern that minimizes average queueing delay is the most regular (deterministic) one — packets arriving at perfectly equal intervals. Random arrivals always produce higher average delay than periodic arrivals at the same rate.

### Property 3: Little's Result

One of the most elegant results in queueing theory:

```
L = λ × d

L = average number of items in the system
λ = average arrival rate (items per second)
d = average time each item spends in the system
```

This holds for *any* stable queue, regardless of the arrival distribution, service distribution, or number of servers. If you know any two of these quantities, you immediately know the third.

### The M/M/1 Queue

The **M/M/1 queue** has Poisson arrivals (M), exponential service times (M), and one server (1). The Poisson process models the aggregation of many independent random events. Network traffic itself is very bursty and not well modeled by Poisson packet arrivals — but the arrival of new flows (new TCP connections, new web requests) is reasonably well approximated by Poisson.

> **Key Takeaway:** Burstiness is the enemy of low delay. Smooth, deterministic arrivals produce the lowest possible queueing delay. Little's Result (L = λd) is a universal law connecting queue occupancy, throughput, and delay — use it to check your intuitions about any system where things wait in line.

---

## Section 07 — How a Packet Switch Works

Every packet switch performs two fundamental operations: address lookup and switching.

### Ethernet Switches vs. Internet Routers

Both are packet switches, but they differ in what they look at and how they look it up.

```python
# Ethernet switch — forwarding logic
def process_ethernet_frame(frame, input_port):
    # Learn: record which port this source MAC came from
    forwarding_table[frame.src_mac] = input_port

    if frame.dst_mac in forwarding_table:
        output_port = forwarding_table[frame.dst_mac]
        forward(frame, output_port)
    else:
        # Unknown destination: flood to all ports except source
        broadcast(frame, exclude=input_port)
```

*Ethernet switches self-learn: they build their forwarding table by watching which MAC addresses arrive on which port. Unknown destinations are broadcast ("flooded") to everyone.*

```python
# Internet router — forwarding logic
def process_ip_packet(packet):
    # Step 1: Accept only frames addressed to this router
    if packet.eth_dst != this_router.mac:
        drop(packet); return

    # Step 2: Decrement TTL to prevent routing loops
    packet.ttl -= 1
    update_checksum(packet)
    if packet.ttl == 0:
        drop(packet); return

    # Step 3: Longest-prefix match on destination IP
    next_hop = forwarding_table.longest_prefix_match(packet.ip_dst)

    # Step 4: Rewrite Ethernet header for next hop and send
    next_mac = arp_table.lookup(next_hop)
    packet.eth_src = this_router.mac
    packet.eth_dst = next_mac
    send(packet, next_hop)
```

*Routers must manage the TTL field (decremented at each hop to kill looping packets) and rewrite the Ethernet header at each hop — while keeping the IP header intact all the way to the destination.*

### Address Lookup: Exact Match vs. Longest Prefix Match

Ethernet switches perform **exact match** lookups — they look for the precise 48-bit MAC address in a hash table.

IP routers perform **longest prefix match**. An IP forwarding table contains entries like "send everything starting with 128.9.0.0/16 to port 3." Multiple prefixes can match a given destination; the router picks the longest (most specific) match. This is implemented in hardware using binary tries or Ternary Content Addressable Memory (TCAM), which can compare a destination address against every entry simultaneously in a single clock cycle.

### Output Queueing vs. Input Queueing

**Output Queued (OQ) switches** have a queue at each output port. They are work-conserving, maximize throughput, and minimize delay — but require the switch fabric to operate at N times the port rate, which becomes impractical at high speeds.

**Input Queued (IQ) switches** have queues at each input port and only require the fabric to run at line rate. However, they suffer from **head-of-line (HOL) blocking**: a packet at the front of an input queue blocks all packets behind it — even if those packets could go to idle output ports. HOL blocking limits throughput to about 58% of maximum.

**Virtual Output Queues (VOQs)** solve this: maintain a separate queue at each input for each output port. Now a slow output port can't block packets destined for faster ones. VOQ switches nearly match the throughput of output-queued switches at realistic loads.

> **Key Takeaway:** Packet switches perform address lookup (exact match for Ethernet, longest-prefix match for IP) and then physically move the packet to the right output. Output queueing gives the best delay performance but is expensive at high speeds; input queueing with virtual output queues is the practical compromise used in high-speed routers.

---

## Section 08 — Rate Guarantees: Giving Each Flow Its Fair Share

In a plain FIFO queue, every packet competes equally for the output link. There's no way to promise any particular flow a minimum rate.

### Strict Priority Queues

Replace the single FIFO queue with two queues: high-priority and low-priority. The server always empties the high-priority queue first; it only serves the low-priority queue when the high-priority queue is completely empty.

High-priority traffic is completely unaffected by low-priority traffic. The downside: if high-priority traffic is heavy, low-priority traffic may never get served (starvation).

### Weighted Fair Queueing (WFQ)

Assign each queue a weight wᵢ. The share of the link given to flow i is:

```
rate_i = (wᵢ / Σwᵢ) × R
```

WFQ's elegant solution for variable-length packets: *pretend* we serve bits rather than packets. Compute, for each arriving packet, when it *would* have finished service under this ideal bit-by-bit scheduler. Then serve packets in the order of these computed finish times.

```
When packet k arrives to queue i:
    F_k = max(F_{k-1}, now) + L_k / w_i

F_k  = finishing round (when it would finish under ideal scheduler)
L_k  = length of packet k (bits)
w_i  = weight of queue i

Serve packets in order of increasing F_k.
```

```python
# WFQ finish-time calculation
class WFQ_Queue:
    def __init__(self, weight):
        self.weight = weight
        self.last_finish = 0

    def enqueue(self, packet, current_round):
        start = max(self.last_finish, current_round)
        packet.finish_round = start + packet.length / self.weight
        self.last_finish = packet.finish_round
        global_priority_queue.insert(packet)   # sorted by finish_round

# Scheduler always dequeues the packet with smallest finish_round
def schedule():
    return global_priority_queue.pop_min()
```

*Each queue computes a virtual finish time for new packets. The global scheduler always picks the packet with the smallest finish time across all queues — guaranteeing proportional rates over the long run regardless of packet sizes.*

This approach is also called Packetized Generalized Processor Sharing (PGPS). Over the long run, WFQ guarantees each flow exactly its weighted share of the link rate.

> **Key Takeaway:** A plain FIFO queue gives bandwidth to whoever sends the most. Weighted Fair Queueing assigns each flow a weight and guarantees it a proportional share of the link. By computing virtual finish times for variable-length packets, WFQ achieves fair, guaranteed rates without requiring all packets to be the same size.

---

## Section 09 — End-to-End Delay Guarantees

### Bounded Delay in a Single Router

A flow served by a WFQ queue with rate rᵢ and a buffer of size B bits waits at most B/rᵢ seconds before being served. If we know the maximum queueing delay at each router along the path, we can add them up to bound the end-to-end delay.

### Traffic Shaping: The Leaky Bucket

To prevent buffer overflow, a **leaky bucket** (token bucket) constrains the traffic generated by a source. In any time interval T, the source may send at most B + ρ·T bits:

```
Traffic constraint:   A(t + T) − A(t) ≤ B + ρ·T    for all t, T ≥ 0

B = burst size (bits)
ρ = sustained rate (bits/second)
```

This is called a (σ, ρ)-constrained arrival process. It allows short bursts up to B bits but limits the long-term average rate to ρ.

### End-to-End Delay Formula

The source shapes its traffic with parameters (B, ρ). Each router puts the flow into a WFQ queue served at rate rᵢ with buffer size B. Because the arrival process is (B, ρ)-constrained and rᵢ ≥ ρ, the queue never overflows:

```
max_delay = Σᵢ (packetization_delay_i) + Σᵢ (propagation_delay_i) + Σᵢ (B / rᵢ)
```

This guarantee is coordinated by **RSVP** (Resource Reservation Protocol, IETF RFC 2205), which signals the required rates and buffer sizes to every router along the path before the flow begins.

### Worked Example

Two hosts are separated by three routers and four 250 km links running at 100 Mb/s. They send 1,500-byte packets at 15 Mb/s and need end-to-end delay under 10 ms.

```python
# Given
link_rate     = 100e6    # 100 Mb/s
link_length   = 250e3    # 250 km in meters
prop_speed    = 2e8      # 2×10⁸ m/s
packet_size   = 1500 * 8 # 12,000 bits
n_links       = 4
n_routers     = 3
max_e2e_delay = 10e-3    # 10 ms
flow_rate     = 15e6     # 15 Mb/s (WFQ rate at each router)

# Fixed delay components
pack_delay  = n_links * (packet_size / link_rate)    # = 0.48 ms
prop_delay  = n_links * (link_length / prop_speed)   # = 5.00 ms
fixed_delay = pack_delay + prop_delay                # = 5.48 ms

# Available queueing budget
queue_budget = max_e2e_delay - fixed_delay           # = 4.52 ms
per_router   = queue_budget / n_routers              # = 1.507 ms each

# Buffer size needed at each router
buffer_bits  = flow_rate * per_router                # ≈ 22,605 bits
buffer_bytes = buffer_bits / 8                       # ≈ 2,826 bytes
# Round up to 2 packets = 3,000 bytes
```

*The fixed delay consumes 5.48 ms of the 10 ms budget. The remaining 4.52 ms is split equally among the three routers — about 1.5 ms per router, requiring roughly 2,826 bytes of buffer per router.*

> **Key Takeaway:** End-to-end delay guarantees require two things working together: traffic shaping at the source (leaky bucket) to prevent buffer overflow, and WFQ scheduling at each router to ensure each flow gets its reserved rate. With both in place, every router's queueing delay is bounded, and the sum of all delays along the path can be guaranteed never to exceed a target.

---

## Summary — The Five Things to Remember

**01 — Packet switching is the foundation of the Internet.**
Rather than dedicating a circuit, data is broken into addressed packets that hop independently through shared links. This enables statistical multiplexing — efficient sharing — and robustness, but introduces variable queueing delay.

**02 — End-to-end delay has three parts.**
Propagation delay (light speed, link length), packetization delay (packet size ÷ link rate), and queueing delay. Only queueing delay is unpredictable — it depends on other users' traffic in shared buffers along the path.

**03 — Streaming apps absorb variable delay with a playback buffer.**
By pre-buffering content before playback begins, applications shield users from temporary network jitter. If the buffer empties, playback freezes — a rebuffering event. Buffer sizing is a deliberate engineering tradeoff between latency and resilience.

**04 — Burstiness is the enemy; Little's Result is universal.**
Bursty arrivals always produce higher average queueing delay than smooth arrivals at the same rate. Little's Law (L = λd) connects average queue occupancy, arrival rate, and delay for any stable queue — a powerful debugging and design tool.

**05 — Rate and delay guarantees require WFQ + traffic shaping.**
Weighted Fair Queueing gives each flow a guaranteed share of the link. A leaky bucket at the source caps the arrival rate, preventing buffer overflow. Together, they bound the queueing delay at each router and enable provable end-to-end delay guarantees for real-time applications.
