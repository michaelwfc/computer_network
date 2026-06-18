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

### The Telephone and Circuit Switching (电路交换)

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

**Circuit Switching(电路交换):**
- Dedicated circuit reserved end-to-end
- Guaranteed bandwidth and delay
- Resources wasted when line is idle
- State maintained in every switch
- Setup phase required before data flows

**Packet Switching(分组交换):**
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

### Propagation Delay $t_l$

**Propagation delay** is the time it takes a single bit to travel from one end of a link to the other. It depends only on the length of the cable(l) and the propagation speed(c) at which signals travel through that medium — roughly the speed of light.


```
propagation_delay = link_length / propagation_speed
```
$t_l = l/c$

Example: 300 km fiber link at 2×10⁸ m/s → 1.5 ms

Crucially, propagation delay does *not* depend on the data rate of the link. A bit travels just as fast on a 1 kb/s link as on a 100 Gb/s link — the speed of light doesn't care how wide the pipe is.

### Packetization Delay $t_p$

**Packetization delay** (also called transmission delay) is the time from when the first bit of a packet is placed onto the link until the last bit is sent. 
Think of it like the time it takes to pour a glass of water — it depends on the size of the glass (packet size) and how fast you pour (data rate).

```
packetization_delay = packet_size_in_bits / link_data_rate
```
$t_p = p/r$


Example: 12,000-bit packet on 100 Mb/s link → 0.12 ms


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
Diagram: A graph showing cumulative bytes on the y-axis and time on the x-axis. 

- The A(t) curve climbs steeply during a burst then levels off; 
- the D(t) curve climbs more gently at the constant service rate R. 
- The vertical gap between the two curves at any time t is the queue occupancy Q(t). 
- The horizontal gap between them at a given height is the queueing delay d(t) experienced by a byte arriving at that time.

### Worked Example

At the start of every second, a train of  100 bits arrives to a queue at rate 1000 bits/s. The departure rate from the queue is 500 b/s. The queue is served bit-by-bit, and you can assume the buffer size if infinite.

#### a. What is the average occupacy of the query?

![image](../../../images/packet_switching/worked-example.png)


What's Happening Physically

```text
arrival_rate   = 1000  # bits/s during the burst
departure_rate   = 500   # bits/s

# 100 bits arrive at 1000 b/s, take 0.1s to arrrive
100 b/arrival_rate = 100/1000 = 0.1 s

# departure rate at 500 b/s, 100 bits take 0.2s in order to depart
100 b/departure_rate = 100/500 = 0.2 s

Great question. Let me walk through the geometry carefully.

t = 0.0s:  packet starts arriving at 1000 b/s
t = 0.1s:  packet fully arrived (100 bits in queue), departures also started at 500 b/s
t = 0.2s:  queue fully drained
t = 1.0s:  next packet arrives
```

During `[0, 0.1s]`: arrivals at 1000 b/s, departures at 500 b/s → queue **grows** at net 500 b/s
During `[0.1s, 0.2s]`: no more arrivals, departures at 500 b/s → queue **shrinks** at 500 b/s

So the queue occupancy Q(t) over time looks like this:

```text
Q(t)
 50 |        *
    |      /   \
 25 |    /       \
    |  /           \
  0 |/_______________\________
    0  0.1  0.2  ...  1.0    t(s)
         ↑     ↑
       peak   empty
```

It's a **triangle**.

---

The Peak Value

At `t = 0.1s`, the queue has been accumulating at net 500 b/s for 0.1 seconds:

```
Q(0.1) = (1000 - 500) × 0.1 = 50 bits   ← peak
```

Then from `t = 0.1s` to `t = 0.2s`, it drains from 50 → 0.

---


The code computes the **average occupancy over the first 0.2 seconds**, not just the rising phase.

The full shape over `[0, 0.2s]` is a triangle:
- Base = 0.2s
- Peak = 50 bits at t = 0.1s (the midpoint)

The average value of a triangle over its base is:

```
average = (1/2) × peak = 50 / 2 = 25 bits
```

Now look at the code expression again:

```python
avg_occupancy_first_0_2s = 0.5 * (0.1 * 500)
```

Breaking it down:
- `0.1 * 500` = net accumulation rate × time of growth = **50 bits** (the peak)
- `0.5 *  50` = **25 bits** (average of the triangle)

So `0.5 × (0.1 × 500)` is just `½ × peak`, which is the average height of a symmetric triangle.

---

Then the Time-Weighted Average

```python
time_avg_occupancy = (0.2 * 25) + (0.8 * 0)  = 5 bits
```

This weights by the fraction of the 1-second cycle each phase occupies:

```text
[0, 0.2s]  → 20% of the cycle, average occupancy = 25 bits
[0.2, 1.0s]→ 80% of the cycle, average occupancy =  0 bits

Overall = 0.2 × 25 + 0.8 × 0 = 5 bits
```

This is a **time-average** (also called time-weighted average), which is exactly what "average queue occupancy" means in queueing theory — the integral of Q(t) over one full period, divided by the period length.

*The queue fills for 0.1 s, drains for 0.2 s, then sits idle for 0.8 s. The time-weighted average occupancy is just 5 bits — much lower than the 50-bit peak.*


#### b. What is the average delay experienced by a bit arriving to the queue?

```
The delay seen by a bit arriving at time t is d(t), the horizontal distance between the two lines.

The first bit arriving at time zero experiences no delay. 
Whereas a bit arriving at 0.1s experiences a delay of 0.1s. 
Notice that no more bits arrive after 0.1s, so it makes no sense to consider the delay of a bit arriving between 0.1s and 1.0s. We are conditioning the probability on a bit arriving, which only happens in the first 0.1 of every second.
Therefore, the average delay seen by a bit arriving to the queue is simply 0.05s.

averay delay seen by a bit arriving to the queue = 1/2 * 0.1= 0.05 s

```

#### c. If the trains of 100 bits arrived at random intervals, one train per second on average, would the average queue occupancy be the same, lower or higher than in part(a) ?


##### Deterministic vs Random Arrivals

###### Part (a) — Deterministic (what you already solved)

The trains arrive on a **perfectly regular schedule**:

```text
Train 1:  [t = 0.0s,  t = 0.1s]
Train 2:  [t = 1.0s,  t = 1.1s]
Train 3:  [t = 2.0s,  t = 2.1s]
...
```

Every train arrives **exactly** 1 second apart. Perfectly predictable. The queue behavior repeats identically every second.

---

###### Part (c) — Random Intervals (the new question)

Now the trains still arrive **on average once per second**, but the gaps between them are **random**. Sometimes two trains arrive close together, sometimes far apart.

```text
Possible realization:

Train 1:  [t = 0.00s,  t = 0.10s]
Train 2:  [t = 0.15s,  t = 0.25s]  ← arrived very soon after train 1
Train 3:  [t = 1.80s,  t = 1.90s]  ← long gap
Train 4:  [t = 2.05s,  t = 2.15s]  ← arrived very soon after train 3
...
```

The **average rate is still 100 bits/s** (same as before), but the **timing is unpredictable**.

---

##### Why This Changes the Queue Occupancy

This is the key insight the question is testing.

In the deterministic case, the queue **always has time to fully drain** before the next train arrives. The 0.8s idle gap guarantees this:

```text
Deterministic:
Q(t)
50|    /\
  |   /  \
  |  /    \_________ (fully drained, 0.8s idle)
  | /
  0──────────────────── t
     0.1 0.2        1.0
```

In the random case, sometimes the next train arrives **before the queue has drained**:

```text
Random (bad luck scenario):
Q(t)
50|    /\  /\
  |   /  \/  \        ← second train arrived before queue drained
  |  /        \___
  | /
  0──────────────────── t
```

Case 1: Assume the trains arrive randomly, but no two trains ever overlap. The average queue occupancy would be the same as before, which was 5bits.

Case 2: As soon as two trains overlap, even once, the average will increase. To see why, consider the example here. The red line shows the queue occupancy of tone train which we assume arrives at time 0. Let’s say the second train arrives at time 0.1s just when the queue has 50bits in it still from the 1sttrain. The queue will keep growing because bits are arriving twice as fast as they are leaving. The queue won’t drain until time 0.4s. If this happened every two seconds, the arrival rate would be the same as before, but the time average queue occupancy would now be as follows.
For 0.4s the time average occupancy is 50b, then for 1.6s it is empty. TO get the time average, we divide by two seconds. The time average occupancy is 10bits, which is double what it was before.

Why is that? It’s because the queue only drains at rate 500bits/second and so not only does it fill to twice as much as before, it also takes twice as long to drain. The triangle showing when the queue is non-empty has four times the area as before.




When trains pile up like this, the queue builds to a **higher level** than in the deterministic case. And crucially, the lucky cases (long gaps) cannot compensate symmetrically — the queue can drain to zero but **no lower**. This asymmetry is called the **waiting room floor effect**: you can't have negative queue occupancy.

```text
Long gap scenario:
Q(t)
  |    /\
  |   /  \___
  |  /        0  0  0  0  ← already empty, extra idle time wasted
  | /
  0──────────────────── t
```

Extra idle time beyond what's needed to drain gives you nothing. But insufficient idle time causes queue buildup. The outcomes are **asymmetric**.

---

##### The Answer

The average queue occupancy with random arrivals is **higher** than in the deterministic case.

This is a fundamental result in queueing theory — **randomness always increases average queue occupancy** compared to the equivalent deterministic system with the same average load. This is formalized in the **Pollaczek-Khinchine (P-K) formula** for M/G/1 queues, but the intuition is exactly the asymmetry above: the floor at zero means bad luck hurts more than good luck helps.

This is also why **traffic shaping** (making arrivals more regular/deterministic) is a real technique in network engineering — it reduces queueing delay without changing the average load.
---


### Why Break Messages Into Packets?

```
One big packet:      t = M/rᵢ + Σ lᵢ/c       (sequential transmission per hop)
Many small packets:  t = p/rᵢ + Σ lᵢ/c + (M/p − 1) × p/rₘᵢₙ   (pipelined)
```

The pipelined version finishes much sooner because intermediate links are kept busy with subsequent packets while early packets move ahead.

---

### Statistical Multiplexing


#### Intuition First

Imagine a highway with one lane merging from 10 side roads.

If every car from every side road drove simultaneously, the highway would jam. But in reality, cars arrive at **different times, in bursts, with idle gaps**. The highway handles the load comfortably on average — even though the *peak* combined rate would overwhelm it.

Statistical multiplexing is exactly this: **exploiting the fact that users don't all burst at the same time**.

---

#### The Core Problem It Solves

Suppose you have 10 users, each needing up to 1 Mbps, sharing one 1 Mbps output link.

**Naive (circuit switching) thinking:**
> Reserve 1 Mbps per user → need 10 Mbps link. Expensive and wasteful.

**Statistical multiplexing thinking:**
> Each user is only active ~10% of the time. On average, only ~1 user is active at once. A 1 Mbps link is sufficient *most of the time*.

```text
Time →
User 1:  ████░░░░░░░░░░████░░░░░░░░
User 2:  ░░░░████░░░░░░░░░░████░░░░
User 3:  ░░░░░░░░████░░░░░░░░░░████
...
User 10: ░░██░░░░░░░░████░░░░░░░░░░

Combined: rarely all active simultaneously
Link:     1 Mbps handles it statistically
```

---

#### The A(t)/D(t) View

Bring back the cumulative arrival/departure model:

```text
Bytes
  ▲
  │        /← burst from User 2
  │       /
  │  A(t)/← burst from User 1
  │    /╲
  │   /  ╲___
  │  /        ╲← drain at link rate R
  │ / D(t)______╲___________
  └─────────────────────────► t
```

- When combined arrivals exceed link rate R → **queue builds up** (buffer absorbs the burst)
- When combined arrivals are below R → **queue drains**
- Long-run stability requires: **average arrival rate < R**

The buffer is the mechanism that makes statistical multiplexing work. It absorbs temporary overload so the link doesn't have to be provisioned for the worst case.

---

#### Why It Works: The Statistics

Let each user be active independently with probability `p`. With `n` users each needing rate `r` when active:

```text
Expected simultaneous active users = n × p
Peak possible simultaneous users   = n        (all burst at once)

Circuit switching provisions for:   n × r
Stat-mux provisions for:       ≈   n × p × r  (plus some margin)
```

**Statistical multiplexing gain** = `n / (n×p)` = `1/p`

If p = 0.1 (10% active), gain = 10×. You serve 10 users with the bandwidth one user would need under circuit switching.

---

#### The Queueing Picture in Detail

Here's what happens moment-to-moment on a shared link:

```text
           ┌─────────────────────────────┐
Flow A ───►│                             │
Flow B ───►│   Shared Output Buffer      ├──► Link (rate R)
Flow C ───►│                             │
Flow D ───►│                             │
           └─────────────────────────────┘
                        ↑
              packets wait here when
              combined input > R
```

Three regimes:

```text
Case 1: Combined arrival rate < R
        → Queue stays empty, no delay, all packets forwarded immediately

Case 2: Combined arrival rate > R  (temporary burst)
        → Queue grows, packets experience queueing delay
        → If burst is short, queue drains afterward

Case 3: Combined arrival rate > R  (sustained overload)
        → Queue grows without bound → buffer fills → PACKET DROP
```

Case 3 is the fundamental tradeoff: **statistical multiplexing gain comes at the cost of possible packet loss and variable delay**.

---

#### The Tradeoff Triangle

```text
              Statistical Multiplexing Gain
                         ▲
                         │
          More users ────┼──── cheaper per-user cost
          sharing one    │     BUT...
          link           │
                         │
            ┌────────────┴─────────────┐
            │                          │
       Higher            Higher variable
       loss probability  queueing delay
       (buffer overflow) (bursts fill queue)
```

You can tune this tradeoff with:
- **Larger buffers** → absorb bigger bursts, less loss, but more delay
- **Higher link rate** → less congestion, lower delay, more expensive
- **Fewer users per link** → less contention, less gain

---

#### Contrast With Circuit Switching

```text
                  Circuit Switching      Packet / Stat-mux
─────────────────────────────────────────────────────────
Bandwidth per user   Guaranteed             Best-effort
Idle bandwidth       Wasted (reserved)      Reclaimed by others
Delay                Fixed, predictable     Variable
Loss                 None (if circuit up)   Possible (buffer overflow)
Provisioning         For peak               For average + margin
Scalability          Low (O(n) resources)   High (shared resources)
```

Circuit switching is like **reserving a table at a restaurant**. Your table sits empty while you're not eating, but you're guaranteed a seat.

Statistical multiplexing is like a **walk-in restaurant with a waiting area**. Most times you're seated immediately. Occasionally you wait. Rarely, if it's completely packed, you're turned away (packet drop).

---

#### Why the Internet Is Built on This

1. **Traffic is inherently bursty.** HTTP, video, keystrokes — all have idle gaps. Reserving for peak is massively wasteful.
2. **The law of large numbers helps you.** With many independent users, combined traffic is far smoother than any individual flow. Variance shrinks relative to the mean.
3. **No per-connection state in routers.** Routers just forward packets. No resource reservation, no setup. This is why the Internet scaled to billions of devices.

The cost — variable delay and occasional loss — is exactly why **TCP exists**: to detect loss and retransmit, and to perform congestion control so the network doesn't collapse under overload. Which is precisely what CS144 has you implement.



When many flows share a single output link, the link doesn't need to run at the sum of all their rates. Because different users are active at different times, a buffer absorbs the brief periods when combined traffic exceeds the link rate. This is **statistical multiplexing gain**. 

The tradeoff: a finite buffer means packets may be dropped if too many users burst at once.

> **Key Takeaway:** 
> - The A(t)/D(t) model gives us a concrete way to reason about queue occupancy and delay. 
> - Breaking messages into packets enables pipelining,and reduce end to end delay
> - Statistical multiplexing lets one shared link efficiently carray many flows




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



### The Poisson process

Why the Poisson process?
Models aggregation of many independent random events, e.g.
- Arrival of new phone calls to a telephone switch
- Decay of many independent nuclear particles
- "Shot noise" in an electrical circuit

It makes the math easy.

**Be warned:**
1. Network traffic is very bursty!
2. Packet arrivals are not Poisson.
3. But it models quite well the arrival of new flows.


#### What is a Poisson Process?
A Poisson process is a mathematical model for random arrivals over time.

Imagine packets arriving at a router:
```
time ------------------------------------------------>

packet arrivals:

      x     x  x        x        x
------|-----|--|--------|--------|---->

```

The arrivals happen:
- randomly
- independently
- at some average rate λ (lambda)

#### Properties of a Poisson Process

Independence : Arrivals in different time intervals are independent.



### The M/M/1 Queue

Two processes are happening simultaneously:

1. New packets arrive.
2. Existing packets are served and depart.

```
Packets Arrive
      ↓
+-------------+
|    Queue    |
+-------------+
      ↓
+-------------+
|   Server    |
+-------------+
      ↓
Packets Leave
```

The **M/M/1 queue** has Poisson arrivals (M), exponential service times (M), and one server (1). The Poisson process models the aggregation of many independent random events. Network traffic itself is very bursty and not well modeled by Poisson packet arrivals — but the arrival of new flows (new TCP connections, new web requests) is reasonably well approximated by Poisson.



> **Key Takeaway:** Burstiness is the enemy of low delay. Smooth, deterministic arrivals produce the lowest possible queueing delay. Little's Result (L = λd) is a universal law connecting queue occupancy, throughput, and delay — use it to check your intuitions about any system where things wait in line.


#### M/M/1 means:

| Symbol   | Meaning                               |
| -------- | ------------------------------------- |
| First M  | Markovian arrivals (Poisson)          |
| Second M | Markovian service times (Exponential) |
| 1        | One server                            |

Arrivals --> Queue --> Server



#### 1st M: Markovian arrival process

The first M actually means: Markovian arrival process
and a Poisson arrival process is a special Markovian process.

In queueing theory: 1st M means Interarrival times are exponentially distributed.

This is equivalent to saying: Arrivals form a Poisson process.

Therefore:
Poisson arrivals ⇔ Exponential interarrival times ⇔ Markovian arrival process

In queueing theory these three descriptions are essentially interchangeable.

##### Why does a Poisson process produce exponential interarrival times?

A Poisson process is defined by:
Number of arrivals in interval length t being Poisson distributed:

$P(N(t) = k) = (λt)^k * e^{(-λt)}/k!$

Now ask: What is the probability that the waiting time until the next arrival exceeds t?
If waiting time exceeds t, then: No arrivals occurred during [0,t]

Therefore $P(T>t) = P(N(t) = 0) $ 

For a Poisson process:  $P(T>t) = P(N(t) = 0) = e^{(-λt)}$ 
which is exactly the exponential survival function.

Therefore: Poisson arrivals ⇔ Exponential interarrival times



##### Why Is It Called Markovian?
Because exponential distributions have the memoryless property.

Suppose: T = time until next arrival

$ P(T > s+t | T > s) = P(T > t) $

- $P(T > s+t | T > s)$: Given that I've already waited s seconds, what is the probability I must wait another t seconds?
- $P(T > t)$: The probability that we must wait more than t time units for the next arrival.

This means: Given that I've already waited 5 seconds,For an exponential distribution, the distribution of the remaining waiting time is unchanged. The previous 5 seconds don't matter.

The process has no memory of the past.

That is exactly the Markov property.




##### Interarrival times vs Arrival counts

Suppose packets arrive like this:

```
time ------------------------------------------------>

packet arrivals:

      x         x      x            x
------|---------|------|------------|----->

The interarrival time is the time between consecutive arrivals.

      x         x      x            x
------|---------|------|------------|----->

        T1        T2        T3

where
T1 = time from packet 1 to packet 2
T2 = time from packet 2 to packet 3
T3 = time from packet 3 to packet 4

These are called interarrival times.
```

Why Queueing Theory Prefers "M"

Queueing analysis is usually easier using interarrival times, rather than arrival counts


For M/M/1 we need:
- arrival intervals ~ Exponential(λ), 
- service times ~ Exponential(μ), 

Survival Function $P(T > t)$ : Probability we're still waiting after time t

$P(T > t) = e^{(-λt)}$ 


**Both are memoryless.**
Therefore the queue only needs to remember: `Current queue length = n`
contains all information needed about the future.

We don't need:: 
- the arrival process doesn't remember how long it's been since the last arrival.
- The server doesn't remember how long it has already been busy.
- When the last arrival occurred

The state `n packets` is enough.

This allows the queue length process to become a **continuous-time Markov chain**:

0 packets
1 packet
2 packets
3 packets
...

with transitions:

n ----λ----> n+1   (arrival)

n ----μ----> n-1   (departure)

This Markov structure is the reason the model is solvable.


Arrivals: New flow arrivals occur according to a Poisson process.

Equivalently: Time between arrivals is exponential.




----

#### 2nd M: What Does Markovian Exponential Service Time Mean?
Suppose a router processes packets.
**Service time** means: How long does one packet occupy the server?

Example: 
- Packet arrives
- Router forwards it
- Packet leaves

The processing time is the service time.


Markovian service times

Meaning: Memoryless
- Already waited 5 ms?
- Remaining wait time distribution is unchanged.
This makes analysis much easier.


An exponential distribution means:

- Most jobs are short
- Some jobs are long
- A few jobs are very long


#### Arrival Rate λ and Service Rate μ

- Arrival rate: λ packets/sec
means: Average number of arrivals per unit time.
Units: packets/sec，  jobs/sec， requests/sec ，  flows/sec， depending on what is being modeled.

- Service rate: μ packets/sec
means: Average number of packets (or jobs) that can be completed per second.

Service Rate μ: Link Capacity/Packet Size
Ex: Packet Size = 10,000 bits ,Link Capacity = 100 Mbps
Service Rate μ = 100 Mbps/ 10,000 bits= 10，000 packet/sec

####  Why Utilization ρ < 1 Is Critical



**Traffic intensity/Utilization**(read: rho): 
ρ = λ / μ  

λ<μ is required for a stable M/M/1 queue.


Example:
λ = 800 packets/sec
μ = 1000 packets/sec
ρ = 800 / 1000 = 0.8, 80% utilization.


Suppose:
λ = 1200 packets/sec
μ = 1000 packets/sec

Then: ρ = 1.2

More packets arrive than can be served. Queue grows forever:


```
time --->

Queue length

10
20
30
40
50
...
```

Eventually: buffer overflow, packet drops

Therefore: λ < μ is required for stability.



#### Average queue length

The average number of packets in the system is:
$$ L= ρ/1-ρ  , where ρ = λ / μ $$

Notice:
50% utilization -> 1 packet
90% utilization -> 9 packets
99% utilization -> 99 packets

The delay explodes as utilization approaches 100%.

This is one of the most important lessons in networking.


#### Exact formulas for For M/M/1



For M/M/1 we need:
- arrival intervals ~ Exponential(λ), 
- service times ~ Exponential(μ), 

1. Arrival process
$$ T_a \sim Exponential(\lambda) $$
Mean interarrival time: 
$$ E[T_a]=\frac1\lambda $$

Ex: $ \lambda=1000 $ packet/s , then $ E[T_a]=\frac1\lambda = 1 ms$
meaning: One arrival every millisecond on average.

2. Service process
$$ T_s \sim \text{Exponential}(\mu) $$
Mean service time:
$$ E[T_s]=\frac1\mu $$


For M/M/1 we can compute things analytically:

CTMC ⇒ Exact formulas for delay, queue length,
       utilization, and loss probabilities


Average queue length:
$$ L=\frac{\rho}{1-\rho}$$

Average delay:
$$d=\frac{1}{\mu-\lambda}$$

Probability system is empty:
$$ P_0=1-\rho$$


Probability there are exactly n packets:
$$P_n=(1-\rho)\rho^n$$

where $$ \rho = \frac{\lambda}{\mu} $$
All of these come from solving a relatively simple CTMC.

---

---

## Section 07 — How a Packet Switch Works

1. What does a packet switch look like?
2. What does a packet switch do?
- Ethernet switch
- Internet router
3. How address lookup works 
- Ethernet switch
- Internet router

Every packet switch performs two fundamental operations: address lookup and switching.

### Ethernet Switches
Ethernet Switches and Internet Routers
Both are packet switches, but they differ in what they look at and how they look it up.


#### What Problem Does a Switch Solve?

Before switches, Ethernet used a **shared bus** — every device heard every frame. All devices competed for the same wire using CSMA/CD. This was a single **collision domain**: only one device could transmit at a time.

```text
Old: Shared Bus (Hub)
─────────────────────────────────────
  │         │         │         │
  A         B         C         D

A sends → B, C, D all hear it (including collisions)
Only 1 device can talk at once
```

A switch gives **each device its own dedicated segment**:

```text
New: Ethernet Switch
              ┌─────────┐
    A ────────┤ port 1  │
    B ────────┤ port 2  ├──────── each port = full bandwidth
    C ────────┤ port 3  │         no collisions between ports
    D ────────┤ port 4  │
              └─────────┘
```

Each port is its own collision domain. A and C can transmit simultaneously to B and D with no interference.

---

#### What Is an Ethernet Frame?

Before understanding switching, you need to know what arrives at the switch.

```text
Ethernet Frame Structure:

┌──────────┬──────────┬──────┬─────────────────┬─────┐
│  DST MAC │  SRC MAC │ Type │    Payload       │ FCS │
│  6 bytes │  6 bytes │2 byte│  46-1500 bytes   │4 byte│
└──────────┴──────────┴──────┴─────────────────┴─────┘
     ↑            ↑
   DA: where     SA: where
  it's going    it came from
```

- **DA** = Destination Address (MAC) — where the switch needs to send it
- **SA** = Source Address (MAC) — who sent it — used for **learning**
- **FCS** = Frame Check Sequence — error detection

The switch operates **only** on these MAC addresses. It never looks at IP addresses. That is the router's job.

---

#### The Forwarding Table

The switch maintains a table mapping MAC addresses to ports:

```text
Forwarding Table (also called MAC table or CAM table):

┌───────────────────┬──────┬──────────┐
│   MAC Address     │ Port │  TTL     │
├───────────────────┼──────┼──────────┤
│ AA:BB:CC:DD:EE:01 │  1   │  280s    │
│ AA:BB:CC:DD:EE:02 │  2   │  295s    │
│ AA:BB:CC:DD:EE:04 │  4   │  178s    │
└───────────────────┴──────┴──────────┘
```

Each entry says: "frames destined for this MAC address should go out this port."

Entries **expire** (TTL) because devices move — a laptop unplugs from port 2 and reconnects to port 3. Stale entries must not persist forever.

---

#### The Four Operations — Step by Step

1. Examine the header of each arriving frame.
2. If the Ethernet DA is in the forwarding table, forward the frame to the correct output port(s).
3. If the Ethernet DA is not in the table, broadcast the frame to all ports (except the one through which the frame arrived).
4. Entries in the table are learned by examining the Ethernet SA of arriving packets.


##### Step 1: Examine the Header

Every frame that arrives on any port is inspected:

```text
Frame arrives on port 2:

┌──────────────────────────────────────────────┐
│ DA: AA:BB:CC:DD:EE:04  SA: AA:BB:CC:DD:EE:02 │
└──────────────────────────────────────────────┘
  ↑ Where to send it?       ↑ Who sent it?
  (used for forwarding)       (used for learning)
```

---

##### Step 2: Forward If DA Is Known

Switch looks up DA in the forwarding table:

```text
DA = AA:BB:CC:DD:EE:04 → found in table → port 4

Action: send frame out port 4 ONLY

              ┌─────────┐
    A (p1) ───┤         │
    B (p2) ──►│ switch  ├──────────────────────────────►  D (p4)
    C (p3) ───┤         │    frame goes here only
    D (p4) ───┤         │
              └─────────┘

A, C never see this frame. This is the efficiency gain over a hub.
```

This is called **unicast forwarding**.

---

##### Step 3: Broadcast If DA Is Unknown

DA is not in the table. Switch has no idea which port the destination is on.

```text
DA = AA:BB:CC:DD:EE:99 → NOT in table

Action: flood to ALL ports EXCEPT the port frame arrived on

              ┌─────────┐
    A (p1) ◄──┤         │
    B (p2) ──►│ switch  ├──► C (p3)   ← frame flooded here
    C (p3) ───┤         ├──► D (p4)   ← frame flooded here
    D (p4) ───┤         ├──► A (p1)   ← frame flooded here
              └─────────┘
                   ↑
              NOT back to p2 (where frame came from)
```

This is called **flooding**. It is the fallback when the switch doesn't know where the destination is.

The correct destination device will receive the frame and eventually reply — at which point the switch **learns** its location (step 4).

---

##### Step 4: Learn From the Source Address

**Every single arriving frame teaches the switch something**, regardless of what happens to it:

```text
Frame arrives on port 2 with SA = AA:BB:CC:DD:EE:02

Switch records:
"AA:BB:CC:DD:EE:02 is reachable via port 2"

┌───────────────────┬──────┐
│   MAC Address     │ Port │
├───────────────────┼──────┤
│ AA:BB:CC:DD:EE:02 │  2   │  ← just learned
└───────────────────┴──────┘
```

This is **self-learning** — the switch builds its forwarding table automatically with zero configuration. No human types in these entries.

---

#### Full Walkthrough: Fresh Switch, Four Devices

Let's trace a complete scenario. Switch starts with **empty table**.

```text
Devices:
  Port 1: A (MAC = AA)
  Port 2: B (MAC = BB)
  Port 3: C (MAC = CC)
  Port 4: D (MAC = DD)
```

---

**Event 1: A sends a frame to B**

```text
Frame: DA=BB, SA=AA, arrives on port 1

Step 4 (learn):   AA → port 1  ✓ (table now has AA)
Step 1 (lookup):  BB → not in table
Step 3 (flood):   send out ports 2, 3, 4

Table: { AA→1 }
B receives frame (so does C and D, but they discard it — wrong DA)
```

---

**Event 2: B replies to A**

```text
Frame: DA=AA, SA=BB, arrives on port 2

Step 4 (learn):   BB → port 2  ✓ (table now has BB)
Step 2 (lookup):  AA → port 1  found!
Step 2 (forward): send out port 1 ONLY

Table: { AA→1, BB→2 }
C and D never see this frame.
```

---

**Event 3: C sends to D**

```text
Frame: DA=DD, SA=CC, arrives on port 3

Step 4 (learn):   CC → port 3  ✓
Step 1 (lookup):  DD → not in table
Step 3 (flood):   send out ports 1, 2, 4

Table: { AA→1, BB→2, CC→3 }
```

---

**Event 4: D replies to C**

```text
Frame: DA=CC, SA=DD, arrives on port 4

Step 4 (learn):   DD → port 4  ✓
Step 2 (lookup):  CC → port 3  found!
Step 2 (forward): send out port 3 ONLY

Table: { AA→1, BB→2, CC→3, DD→4 }
```

Now the table is fully populated. All future unicast frames are forwarded directly with no flooding.

---

#### Switch vs Router: The Key Distinction

```text
                  Ethernet Switch        IP Router
──────────────────────────────────────────────────────
Layer             Layer 2 (Link)         Layer 3 (Network)
Address used      MAC address            IP address
Table             MAC → port             IP prefix → next hop
Scope             Local network (LAN)    Between networks
Learns            Automatically          Via routing protocols
Forwarding        Hardware (CAM table)   Software or hardware
Broadcasts        Yes (unknown DA)       No (drops broadcasts)
```

The switch is entirely **unaware of IP**. It only sees MAC addresses. The router is entirely **unaware of which physical port** a device is on — it only sees IP addresses.

```text
Your laptop → [Ethernet Switch] → [Router] → Internet
               sees MAC only      sees IP only
               LAN scope          global scope
```

---

#### Special Cases

##### Broadcast Frame (DA = FF:FF:FF:FF:FF:FF)

```text
DA = FF:FF:FF:FF:FF:FF  (broadcast address)

Action: always flood to ALL ports except source port
        even if table is fully populated

Used by: ARP ("who has IP 192.168.1.1?")
         DHCP ("I need an IP address")
```

##### Frame Arrives for Same Port It Would Leave On

```text
Two devices on the same port (via a hub):
  A ──┐
      ├── port 1 ──[switch]
  B ──┘

A sends to B: DA=BB → switch looks up BB → port 1
              source port = port 1 = destination port

Action: DISCARD the frame (don't send it back out same port)
        The hub between them already delivered it.
```

---

#### Why This Design Is Elegant

The switch's self-learning algorithm is a beautiful example of **emergent behavior from simple rules**:

1. Always learn from SA
2. Forward to known DA
3. Flood unknown DA
4. Expire old entries

No configuration. No routing protocols. No administrator intervention. Just four rules applied to every frame, and the switch builds a complete picture of the network topology automatically.

This is why Ethernet switches are **plug-and-play** — you connect cables and they work.



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

### Internet Routers

1. If the Ethernet DA of the arriving frame belongs to the router, accept the frame. Else drop it.
2. Examine the IP version number and length of the datagram.
3. Decrement the TTL, update the IP header checksum.
4. Check to see if TTL == 0.
5. If the IP DA is in the forwarding table, forward to the correct egress port(s) for the next hop.
6. Find the Ethernet DA for the next hop router.
7. Create a new Ethernet frame and send it.


#### The Big Picture: What Is a Router's Job?

A switch connects devices **within** a network (LAN). A router connects **different networks** together and moves packets **across** the internet toward their destination.

```text
Home Network          Internet          Remote Server
192.168.1.0/24                          203.0.113.0/24

[Laptop]──[Switch]──[Home Router]══════[ISP Router]══════[Server]
           LAN         ↑                     ↑
                  Layer 2→3            Layer 3→2
                  boundary             boundary
```

At each router, the packet **sheds its old Ethernet frame** and gets **dressed in a new one** for the next hop. The IP datagram inside is preserved (mostly). The Ethernet wrapper is rebuilt at every hop.

This is the key insight:
```text
IP datagram:    travels end-to-end, source to destination (unchanged mostly)
Ethernet frame: lives only one hop, rebuilt at every router
```

---

#### Step 1: Accept or Drop the Frame

##### What Happens

When a frame arrives on a router's interface, the router first checks the **Ethernet Destination Address**:

```text
Arriving frame:
┌──────────────┬──────────────┬──────┬────────────────┐
│  DA (MAC)    │  SA (MAC)    │ Type │   IP datagram  │
│ AA:BB:CC:..  │ 11:22:33:..  │0x800 │  src→dst IP    │
└──────────────┴──────────────┴──────┴────────────────┘
       ↑
  Is this MY MAC address?
```

```text
Case 1: DA == router's own MAC address
        → Accept frame, strip Ethernet header, process IP datagram

Case 2: DA != router's own MAC address
        → Drop frame silently
           (this frame was not meant for this router)
```

##### Why This Design?

The previous hop already made a **forwarding decision** — it decided this router is the next hop and addressed the Ethernet frame specifically to this router's MAC. If the DA doesn't match, something went wrong. The router is not a switch — it doesn't flood or learn.

```text
Laptop (192.168.1.5) sends packet to 8.8.8.8:

Ethernet frame on the wire:
  DA = router's MAC (AA:BB:CC:DD:EE:01)  ← laptop looked this up via ARP
  SA = laptop's MAC
  payload = IP datagram (src=192.168.1.5, dst=8.8.8.8)

Router sees DA == its own MAC → accepts it ✓
```

---

#### Step 2: Examine IP Version and Length

##### What Happens

After stripping the Ethernet header, the router looks at the **IP header**:

```text
IP Header (IPv4):
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
├───────┬───────┬───────────────────────────────────────────────────┤
│Version│  IHL  │    DSCP   │ECN│          Total Length             │
│  (4)  │  (5)  │           │   │                                   │
├───────┴───────┴───────────────────┬───────────────────────────────┤
│         Identification            │Flags│    Fragment Offset       │
├───────────────────────────────────┴──┬────────────────────────────┤
│      TTL        │    Protocol        │       Header Checksum       │
├─────────────────┴────────────────────────────────────────────────┤
│                       Source IP Address                           │
├──────────────────────────────────────────────────────────────────┤
│                    Destination IP Address                         │
└──────────────────────────────────────────────────────────────────┘
  ↑                    ↑
Version=4 (IPv4)    Total Length = header + payload in bytes
or Version=6 (IPv6)
```

The router checks:
- **Version**: is this IPv4 or IPv6? Different processing paths.
- **IHL** (IP Header Length): where does the header end and payload begin?
- **Total Length**: is the datagram the right size? Does it match what arrived?

If these are malformed → drop the packet.

---

#### Step 3: Decrement TTL and Update Checksum

##### What Is TTL?

**TTL = Time To Live**. It is an 8-bit counter in the IP header, set by the sender (typically 64 or 128). **Every router decrements it by 1** before forwarding.

##### Why TTL Exists

TTL prevents packets from **circling forever** if there is a routing loop:

```text
Without TTL — routing loop:

RouterA thinks: "8.8.8.8 → send to RouterB"
RouterB thinks: "8.8.8.8 → send to RouterC"
RouterC thinks: "8.8.8.8 → send to RouterA"

Packet loops: A→B→C→A→B→C→A→B→C... forever
Network fills with zombie packets
```

With TTL:
```text
TTL starts at 64
RouterA: TTL 64→63, forward to B
RouterB: TTL 63→62, forward to C
RouterC: TTL 62→61, forward to A
...
Eventually TTL = 1
Next router: TTL 1→0 → DROP + send ICMP "Time Exceeded" back to source
```

This is exactly what **traceroute** exploits — it sends packets with TTL=1, 2, 3... and collects the ICMP Time Exceeded messages to map the path:

```text
traceroute 8.8.8.8

TTL=1 → first router drops it  → "192.168.1.1  1ms"
TTL=2 → second router drops it → "10.0.0.1     5ms"
TTL=3 → third router drops it  → "72.14.x.x   10ms"
...
```

##### Why Must Checksum Be Updated?

The IP header checksum covers the **entire IP header**, including the TTL field. When TTL changes, the checksum is now **wrong**. The router must recompute it:

```text
Before:  TTL=64, checksum=0xABCD
Decrement: TTL=63
After:   TTL=63, checksum=0xABCE  ← must recompute
```

This is a performance cost — every router must recompute the checksum on every packet. This is one reason **IPv6 eliminated the header checksum entirely** — lower layers (Ethernet FCS) and upper layers (TCP/UDP checksum) already protect integrity, making the IP checksum redundant and expensive.

---

#### Step 4: Check If TTL == 0

```text
After decrement:

TTL > 0  → continue processing, forward the packet
TTL == 0 → DROP the packet
            Send ICMP Type 11 "Time Exceeded" back to original source IP
```

```text
ICMP Time Exceeded message sent back to source:
┌─────────────────────────────────────────┐
│ Type=11, Code=0                         │
│ "TTL exceeded in transit"               │
│ Includes first 8 bytes of original IP   │
│ header so source can identify the flow  │
└─────────────────────────────────────────┘
```

---

#### Step 5: Forwarding Table Lookup

```
Old frame (arrived on ingress port):
┌──────────────────┬──────────────────┬──────────────────────────┐
│ DA=router MAC    │ SA=laptop MAC    │ IP: src=192.168.1.5      │
│ AA:BB:CC:DD:EE:01│ 11:22:33:44:55:66│      dst=8.8.8.8  TTL=63 │
└──────────────────┴──────────────────┴──────────────────────────┘


```
##### What the Forwarding Table Looks Like

Unlike a switch's MAC table (exact match), a router uses **longest prefix match** on IP prefixes:

```text
Forwarding Table:

┌─────────────────┬──────────────┬───────────┐
│  Prefix         │  Next Hop    │  Egress   │
│                 │  IP          │  Port     │
├─────────────────┼──────────────┼───────────┤
│ 0.0.0.0/0       │ 10.0.0.1     │  eth0     │  ← default route
│ 10.0.0.0/8      │ 10.0.0.1     │  eth0     │
│ 192.168.1.0/24  │ 192.168.1.1  │  eth1     │
│ 203.0.113.0/24  │ 203.0.113.1  │  eth2     │
└─────────────────┴──────────────┴───────────┘
```

##### Longest Prefix Match

For destination IP, the router picks the **most specific matching prefix**:

```text
Destination: 192.168.1.55

Matches:
  0.0.0.0/0       → matches (0 bits must match)
  192.168.1.0/24  → matches (24 bits must match) ← MOST SPECIFIC → use this

Destination: 8.8.8.8

Matches:
  0.0.0.0/0  → matches
  (nothing else matches)
  → use default route 0.0.0.0/0
```

```text
Why longest prefix match?

ISP has: 10.0.0.0/8      → send to backbone
You add: 10.1.2.0/24     → send to your specific subnet

A packet to 10.1.2.5 matches BOTH.
Longest prefix (10.1.2.0/24) wins → correctly routed to your subnet.
Shorter prefix is the fallback for everything else in 10.0.0.0/8.
```

---

#### Step 6: Find the Ethernet DA for the Next Hop

##### The Problem

The forwarding table gives us the **next hop IP address** (e.g., `10.0.0.1`). But Ethernet frames need a **MAC address**, not an IP address.

How does the router find the MAC address of the next hop router?

##### ARP — Address Resolution Protocol

```text
Router needs to send to next hop IP: 10.0.0.1
Router checks its ARP cache:

ARP Cache:
┌─────────────┬───────────────────┬──────────┐
│  IP Address │   MAC Address     │  TTL     │
├─────────────┼───────────────────┼──────────┤
│ 10.0.0.1    │ AA:BB:CC:DD:EE:FF │  180s    │  ← found! use this MAC
│ 10.0.0.2    │ 11:22:33:44:55:66 │  120s    │
└─────────────┴───────────────────┴──────────┘
```

If not in ARP cache:
```text
Router broadcasts ARP Request:
  "Who has IP 10.0.0.1? Tell 10.0.0.2"
  DA = FF:FF:FF:FF:FF:FF (broadcast)

Next hop router replies with ARP Reply:
  "10.0.0.1 is at AA:BB:CC:DD:EE:FF"
  DA = router's MAC (unicast back)

Router caches this mapping, uses the MAC
```

---

#### Step 7: Build New Ethernet Frame and Send

Now the router has everything it needs. 

A Router Has MULTIPLE Interfaces, Each With Its Own MAC
```
Router physical reality:

         eth0                        eth1
    (ingress interface)         (egress interface)
    MAC: AA:BB:CC:DD:EE:01      MAC: 99:88:77:66:55:44
    IP:  192.168.1.1            IP:  10.0.0.2
         │                               │
         │ cable from laptop             │ cable to next router
         │                               │
      [Laptop]                     [Next Router]
      MAC: 11:22:33:44:55:66      MAC: AA:BB:CC:DD:EE:FF
      IP:  192.168.1.5            IP:  10.0.0.?
```

It **discards the old Ethernet frame** and **builds a brand new one**:


```text
Old frame (arrived on ingress port):
┌──────────────────┬──────────────────┬──────────────────────────┐
│ DA=router MAC    │ SA=laptop MAC    │ IP: src=192.168.1.5      │
│ AA:BB:CC:DD:EE:01│ 11:22:33:44:55:66│      dst=8.8.8.8  TTL=63 │
└──────────────────┴──────────────────┴──────────────────────────┘
        ↓  stripped
        ↓  IP datagram extracted
        ↓  TTL decremented, checksum updated
        ↓  new Ethernet frame built

New frame (sent out egress port):
┌──────────────────┬──────────────────┬──────────────────────────┐
│ DA=next hop MAC  │ SA=router's MAC  │ IP: src=192.168.1.5      │
│ AA:BB:CC:DD:EE:FF│ 99:88:77:66:55:44│      dst=8.8.8.8  TTL=62 │
└──────────────────┴──────────────────┴──────────────────────────┘
```

Notice:
- **IP source and destination**: unchanged end-to-end
- **Ethernet DA**: next hop's MAC (changes every hop)
- **Ethernet SA**: this router's outgoing interface MAC (changes every hop)
- **TTL**: decremented by 1
- **IP checksum**: recomputed


- 1. A frame arrives on the router ingress interface(eth0)
When a frame arrives on eth0 in the router(ingress interface),  the router strips Ethernet header. 

- 2. Looks up destination IP & DA
Looks up destination IP → egress = eth1
Finds next hop MAC via ARP → AA:BB:CC:DD:EE:FF


- 3. builds and sends the new frame on the router egress interface(eth1)
Router builds NEW frame to send out eth1.
When it sends the new frame out eth1, the source MAC must be eth1's MAC — because the next router needs to know which MAC to reply to on that specific link.

---

#### The Full Hop-by-Hop Picture

```text
Laptop──────[Router A]──────[Router B]──────[Server]
       eth0        eth1  eth0        eth1
(192.168.1.5)        (8.8.8.8)


Laptop (192.168.1.5)
  → Home Router (192.168.1.1)
  → ISP Router (10.0.0.1 or some public IP)
  → ... more ISP routers ...
  → Google's Router
  → 8.8.8.8 (Google DNS server)

8.8.8.8 is Google's public DNS server. One of the most famous IPs on the internet.

MACs:
  Laptop:           LA
  Router A eth0:    A0   (ingress from laptop)
  Router A eth1:    A1   (egress toward Router B)
  Router B eth0:    B0   (ingress from Router A)
  Router B eth1:    B1   (egress toward Server)
  Server:           S
```


##### Hop 1: Laptop → RouterA (Laptop to Home Router)
  Ethernet: DA=RouterA_eth0_MAC, SA=Laptop_MAC   ← laptop addressed to Router A's eth0
  IP:       src=192.168.1.5, dst=8.8.8.8, TTL=64

The Laptop's Routing Table
Every host (not just routers) has a local routing table. On Linux you can see it with `ip route`:
```bash
$ ip route
default via 192.168.1.1 dev eth0        ← default gateway
192.168.1.0/24 dev eth0 proto kernel    ← local subnet

```
When the laptop wants to send to 8.8.8.8, Laptop checks its own routing table: 

```
Destination: 8.8.8.8

Match against:
  192.168.1.0/24 → does 8.8.8.8 match? NO (different subnet)
  0.0.0.0/0      → does 8.8.8.8 match? YES (matches everything)  → use default gateway: 192.168.1.1
```
The default gateway (0.0.0.0/0) is the fallback: "if you don't know where to send it, send it to me." The home router is that gateway.

But Wait — Laptop Needs the MAC of 192.168.1.1
The laptop knows the next hop IP is 192.168.1.1. But Ethernet needs a MAC address. So the laptop runs ARP:


```
Laptop ARP cache: does it have 192.168.1.1 → MAC mapping?

If NO:
  Broadcast ARP Request:
    DA = FF:FF:FF:FF:FF:FF
    "Who has 192.168.1.1? Tell 192.168.1.5"

  Home Router replies:
    "192.168.1.1 is at AA:BB:CC:DD:EE:01"

  Laptop caches: 192.168.1.1 → AA:BB:CC:DD:EE:01

If YES:
  Use cached MAC directly

```

Now the laptop builds the frame:

```
Ethernet frame leaving laptop:
  DA = AA:BB:CC:DD:EE:01   ← home router's eth0 MAC (from ARP)
  SA = laptop's MAC
  payload: IP datagram src=192.168.1.5, dst=8.8.8.8, TTL=64
```


##### Hop 2: RouterA → RouterB (Home Router Forwards to ISP)
  Ethernet: DA=RouterB_eth0_MAC, SA=RouterA_eth1_MAC   ← new frame
  IP:       src=192.168.1.5, dst=8.8.8.8, TTL=63  ← TTL--


Router receives frame on eth0, extracts IP datagram, looks up dst=8.8.8.8:

Home Router's Forwarding Table
```
┌──────────────────┬───────────────┬────────┐
│ Prefix           │ Next Hop      │ Port   │
├──────────────────┼───────────────┼────────┤
│ 192.168.1.0/24   │ directly conn │ eth0   │ ← local LAN
│ 0.0.0.0/0        │ 10.0.0.1      │ eth1   │ ← ISP gateway
└──────────────────┴───────────────┴────────┘

Longest prefix match for 8.8.8.8:

  192.168.1.0/24  → 8.8.8.8 starts with 8, not 192 → NO MATCH
  0.0.0.0/0       → matches everything              → MATCH

Winner: 0.0.0.0/0 → next hop = 10.0.0.1 via eth1

```






#### Hop 3: RouterB → ... →  Server
  Ethernet: DA=Server_MAC, SA=RouterB_eth1_MAC    ← new frame
  IP:       src=192.168.1.5, dst=8.8.8.8, TTL=62  ← TTL--

IP layer: sees only src=192.168.1.5, dst=8.8.8.8 end-to-end
Ethernet layer: Every hop brand new Ethernet frame, new SA, new DA. 

Let me draw the complete realistic path:


```
Laptop          Home Router      ISP Router 1     ISP Router 2      Google Router     Google DNS
192.168.1.5     192.168.1.1      10.0.0.1         72.14.x.x         216.58.x.x        8.8.8.8
    │                │                │                 │                  │                │
    │                │                │                 │                  │                │
    │──── Hop 1 ────►│                │                 │                  │                │
    │ DA=RouterMAC   │                │                 │                  │                │
    │ SA=LaptopMAC   │                │                 │                  │                │
    │ IP:1.5→8.8 64  │                │                 │                  │                │
    │                │──── Hop 2 ────►│                 │                  │                │
    │                │ DA=ISP1_MAC    │                 │                  │                │
    │                │ SA=Router_eth1 │                 │                  │                │
    │                │ IP:1.5→8.8 63  │                 │                  │                │
    │                │                │──── Hop 3 ─────►│                  │                │
    │                │                │ IP:1.5→8.8 62   │                  │                │
    │                │                │                 │──── Hop 4 ───────►│                │
    │                │                │                 │ IP:1.5→8.8 61    │                │
    │                │                │                 │                  │──── Hop 5 ────►│
    │                │                │                 │                  │ IP:1.5→8.8 60  │
```

At every hop:

Ethernet frame: completely replaced (new DA, new SA)
IP datagram: src and dst unchanged, TTL decremented by 1


---

#### Switch vs Router Side by Side

```text
                    Ethernet Switch          IP Router
────────────────────────────────────────────────────────────────
What it reads       Ethernet DA (MAC)        IP DA (IP address)
Table type          MAC → port               IP prefix → next hop
Table built by      Self-learning (SA)       Routing protocols (OSPF, BGP)
Match type          Exact match              Longest prefix match
Modifies frame?     No                       Yes (new Ethernet frame)
Modifies datagram?  No                       Yes (TTL--, checksum)
Scope               One LAN                  Global internet
Broadcasts          Yes (unknown DA)         No
Knows about IP?     No                       Yes
Knows about MAC?    Yes                      Yes (via ARP, one hop only)
```

---

#### Key Takeaway

The router implements the **boundary between layers**:

```text
Ethernet (Layer 2):  per-hop delivery     → rebuilt at every router
IP (Layer 3):        end-to-end delivery  → preserved across all hops

Router's job: receive L2 frame → extract L3 datagram → 
              make L3 forwarding decision → wrap in new L2 frame → send
```

This separation is why the internet can run over Ethernet, WiFi, fiber, 5G, and satellite simultaneously — IP doesn't care what Layer 2 technology carries it between hops.


```
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

#### Address Lookup: Exact Match vs. Longest Prefix Match

Ethernet switches perform **exact match** lookups — they look for the precise 48-bit MAC address in a hash table.

IP routers perform **longest prefix match**. An IP forwarding table contains entries like "send everything starting with 128.9.0.0/16 to port 3." Multiple prefixes can match a given destination; the router picks the longest (most specific) match. This is implemented in hardware using binary tries or Ternary Content Addressable Memory (TCAM), which can compare a destination address against every entry simultaneously in a single clock cycle.

### Switching  packets to the egress port   


Consider a switch:
```
          Input Ports          Switch Fabric          Output Ports

        100G ----\
        100G -----\               +---+             ---- 100G
        100G ------> ------------>|   |-----------> ---- 100G
        100G -----/               +---+             ---- 100G
        100G ----/

```

- Line Rate (R) : 
Line rate refers to the speed of a physical port.
R = 100 Gbps means: Each port can send or receive 100 billion bits per second.
So: Input port rate  = 100 Gbps , Output port rate = 100 Gbps
Think of line rate as Road speed limit  for a single lane.

- Fabric Speed
The switch fabric is the internal network connecting inputs to outputs.
```
Input Port
     |
     v
+------------+
|   Fabric   |
+------------+
     |
     v
Output Port
```
Fabric speed means: How much traffic the switch can move internally.

Think of the fabric as A highway interchange  inside the switch.
The ports are roads entering and leaving.

#### The Core Problem: Moving Packets From Input to Output

A switch/router has N input ports and N output ports. The fundamental engineering challenge is:

```text
Multiple packets arrive simultaneously on different input ports,
all potentially wanting to go to the same output port.

How do you move them efficiently?

         ┌─────────────────────────┐
Input 1 ─┤                         ├─ Output 1
Input 2 ─┤    Switch Fabric        ├─ Output 2
Input 3 ─┤    (how to cross?)      ├─ Output 3
Input 4 ─┤                         ├─ Output 4
         └─────────────────────────┘

What happens when Input 1, 2, 3 all want Output 1 simultaneously?
```

This is the **contention problem**. The three architectures below solve it differently.

---

#### Architecture 1: Output Queueing (OQ)

**Output Queued (OQ) switches** have a queue at each output port. They are work-conserving, maximize throughput, and minimize delay — but require the switch fabric to operate at N times the port rate, which becomes impractical at high speeds.


##### The Idea

Every packet that arrives is **immediately delivered to the output port** it needs. Queuing happens only at the output.



```text
Incoming packet
      ↓
Switch fabric
      ↓
Output queue
      ↓
Output link


         ┌─────────────────────────┐
         │                         │
Input 1 ─┼──────────────────────►  │ Output 1 [P1][P2][P3] ← queue here
         │      Fabric             │
Input 2 ─┼──────────────────────►  │ Output 2 [P4]
         │   (runs at N×R)         │
Input 3 ─┼──────────────────────►  │ Output 3 []
         │                         │
Input 4 ─┼──────────────────────►  │ Output 4 [P5][P6]
         │                         │
         └─────────────────────────┘
```

##### The Speed Problem

In the worst case, **all N inputs send to the same output in the same time slot**. The fabric must deliver all N packets to that output port simultaneously.

```text
Time slot: 1ms
Port Line rate R: 1 Gbps per port
N ports: 8

Worst case: all 8 inputs send to Output 1

To do this, the switch Fabric must  move 8 packets to Output 1 in 1ms

during the time normally used to transmit 1 packet on a port, Therefore the fabric speed must be: 8 × 1 Gbps = 8 Gbps

For a general N-port switch:
Fabric speed = N × R
```

With N=100 ports each at 100 Gbps → fabric needs 10 **Tbps**. This is physically impractical with current technology.

The fabric must be able to transfer data internally 100 times faster than a single port.
This is called Speedup = N and is very expensive.

##### Why OQ Has Best Delay

Because packets go directly to the output — no waiting at input:

```text
Timeline (OQ):

t=0: P1 arrives on Input1, destined for Output1
t=0: P1 immediately transferred to Output1 queue
t=0: P1 starts being served at Output1

Minimum possible delay. Work-conserving: output port
is never idle if there are packets waiting.
```

---

#### Architecture 2: Input Queueing (IQ)

**Input Queued (IQ) switches** have queues at each input port and only require the fabric to run at line rate. However, they suffer from **head-of-line (HOL) blocking**: a packet at the front of an input queue blocks all packets behind it — even if those packets could go to idle output ports. HOL blocking limits throughput to about 58% of maximum.



##### The Idea

Queue packets at the **input ports** instead. The fabric only needs to run at line rate (1×R), not N×R. Much more practical.
where:
- R = port line rate (e.g., 100 Gbps)
- N = number of ports

The reason OQ needs N×R is that up to N packets may need to be transferred to output queues simultaneously in one time slot, while IQ allows packets to remain queued at inputs, so the fabric only moves packets at normal port speed. This is the fundamental scalability advantage of input-queued switches.

```text
Incoming packet
      ↓
Input Queue
      ↓
Switch fabric
      ↓
Output link



         ┌───────────────────────────────────────┐
         │                                       │
[P1→O1]  │                                       │
[P2→O2]  ├─ Input 1 queue                        │ Output 1
[P3→O1]  │                                       │
         │                       Fabric          │
[P4→O2]  │                     (runs at 1×R)     │ Output 2
[P5→O3]  ├─ Input 2 queue                        │
         │                                       │
[P6→O1]  │                                       │ Output 3
[P7→O3]  ├─ Input 3 queue                        │
         │                                       │
         └───────────────────────────────────────┘
```

Each time slot, a **scheduler** decides which input-output pairs can transfer simultaneously (no two inputs to same output, no two outputs from same input).

Only one packet can leave through Output1 this time slot.
so 1 packet crosses fabric, 2 packets stay queued

Therefore the fabric only needs to move packets at R per port.
Fabric speed = R

No speedup is required. 
The fabric runs at: 1 × R which is called line rate.


##### Head-of-Line (HOL) Blocking — The Fatal Flaw

Here is the problem. Each input has **one FIFO queue**. The packet at the **head of the queue** must go first.

```text
Scenario:

Input 1 queue:  [P1→Output1] [P2→Output2]  ← P1 is at head
Input 2 queue:  [P3→Output1]               ← P3 is at head
Input 3 queue:  [P4→Output2]               ← P4 is at head

Time slot 1:
  Scheduler picks: Input1→Output1 (P1) and Input3→Output2 (P4)
  Cannot pick Input2→Output1 because Output1 already busy

  Input 2 queue:  [P3→Output1]  ← P3 STILL waiting, Output1 busy

Time slot 2:
  Input 1 queue now has: [P2→Output2]
  Input 2 queue still:   [P3→Output1]

  P3 is at the HEAD of Input2's queue.
  P3 wants Output1.
  But suppose Input1's P2 also wants Output2.

  Scheduler picks: Input1→Output2 (P2) and Input2→Output1 (P3)

  This worked this time. But consider:
```

The **catastrophic case**:

```text
Input 1 queue:  [P1→Output1] [P2→Output2] [P3→Output2]
Input 2 queue:  [P4→Output1] [P5→Output2]
Input 3 queue:  [P6→Output2]              ← wants Output2, which is FREE

Time slot 1:
  Scheduler picks Input1→Output1 and... 
  Input3 wants Output2 — Output2 is FREE!
  BUT P1 is being served, so fabric is busy for Input1.
  Input3 CAN go to Output2.

  BUT what if Input3's queue head was:
  [P6→Output1]  ← wants Output1 which is BUSY

  Then P6 blocks even though Output2 is completely idle.
  P6 is stuck behind itself — this is HOL blocking.
```

Let me show it more clearly:

```text
HOL Blocking — the clearest example:

         Input queues          Outputs
         ┌─────────┐
Input 1: │P1→Out1  │──────────► Output 1 ← BUSY (serving P1)
         │P2→Out2  │
         └─────────┘
         ┌─────────┐
Input 2: │P3→Out1  │──────────► Output 1 ← BUSY (can't go)
         │P4→Out2  │   BLOCKED!
         └─────────┘           Output 2 ← IDLE ← nobody can reach it!
         ┌─────────┐                           ↑
Input 3: │P5→Out1  │──────────► Output 1 ← BUSY (can't go)
         │P6→Out2  │   BLOCKED!
         └─────────┘
                    ↑
         P4 and P6 could go to Output2 RIGHT NOW
         but they are stuck behind P3 and P5
         which are stuck waiting for Output1
         
         Output2 sits idle even though packets want it.
         This is HOL blocking.
```

##### The 58% Throughput Limit

HOL blocking was mathematically proven to limit maximum throughput to:

```text
Maximum throughput = 1 - 1/e ≈ 0.632 ≈ 58-63%

Even if you have infinite packets waiting,
even with perfect scheduling,
a simple IQ switch can only use 58% of available bandwidth.

The other 42% is wasted due to HOL blocking.
```

This was proven by Hluchyj and Karol in 1988 and is a fundamental result.

---

#### Architecture 3: Virtual Output Queues (VOQ)


##### The Root Cause and the Fix

HOL blocking happens because **one queue serves all destinations**. A packet stuck waiting for a busy output blocks packets behind it that could go to idle outputs.

The fix: **give each input its own separate queue per output port**.

```text
Without VOQ (one queue per input):

Input 2: [P3→Out1][P4→Out2]  ← P4 blocked behind P3


With VOQ (one queue per input PER output):

Input 2, VOQ for Output1: [P3]     ← P3 waits for Output1
Input 2, VOQ for Output2: [P4]     ← P4 can go NOW, independently
```

##### Full VOQ Architecture

```text
         VOQ structure at each input:

Input 1: VOQ→Out1: [P1]
         VOQ→Out2: [P2]
         VOQ→Out3: []
         VOQ→Out4: [P7]

Input 2: VOQ→Out1: [P3]
         VOQ→Out2: [P4]        ← separate queue, not blocked by P3
         VOQ→Out3: [P8]
         VOQ→Out4: []

Input 3: VOQ→Out1: [P5]
         VOQ→Out2: [P6]
         VOQ→Out3: []
         VOQ→Out4: []
```

Now the scheduler looks across **all VOQs** and finds the maximum set of non-conflicting transfers:

```text
Time slot 1 — scheduler finds:
  Input1 VOQ→Out1: P1 available
  Input2 VOQ→Out2: P4 available  ← no longer blocked by P3!
  Input3 VOQ→Out3: nothing

  Schedule: Input1→Output1 (P1), Input2→Output2 (P4)
  Both transfers happen simultaneously, no conflict.

Time slot 2:
  Input2 VOQ→Out1: P3 now at head, Output1 is free
  Schedule: Input2→Output1 (P3), ...
```

##### Memory Cost of VOQ

VOQ requires N queues per input port, N input ports total:

```text
Total VOQs = N × N = N²

N=8  ports → 64 VOQs
N=64 ports → 4096 VOQs
N=1000 ports → 1,000,000 VOQs

This is manageable in hardware — each VOQ is just a pointer
into a shared memory buffer. The metadata is O(N²) but the
actual packet data is stored once in shared memory.
```

---

##### The Scheduler: Heart of a VOQ Switch

With VOQs, you need a **matching algorithm** — given which VOQs are non-empty, find the maximum set of simultaneous input→output transfers where:
- Each input is used at most once
- Each output is used at most once

This is the **bipartite matching problem**:

```text
Inputs          Outputs
  1 ─────────────► 1
  2 ─────────────► 2
  3 ─────────────► 3
  4 ─────────────► 4

Find maximum matching: maximum number of
simultaneous non-conflicting transfers.

Example:
  Input1→Output3  ✓
  Input2→Output1  ✓
  Input3→Output4  ✓
  Input4→Output2  ✓
  All 4 simultaneously! Perfect matching.
```

Computing perfect bipartite matching is O(N³) — too slow for line rate. Real switches use **approximation algorithms** like iSLIP that run in O(N log N) and achieve near-optimal throughput.

---

#### Side-by-Side Comparison

```text
                OQ Switch         IQ Switch        VOQ Switch
──────────────────────────────────────────────────────────────
Queue location  Output ports      Input ports      Input ports
                                                   (per output)

Fabric speed    N × R             1 × R            1 × R

HOL blocking    No                Yes              No

Max throughput  100%              ~58%             ~100%

Complexity      High (fabric)     Low (fabric)     Medium
                                  High (HOL loss)  (scheduler)

Memory          N output buffers  N input buffers  N² VOQs
                                                   (shared memory)

Used in         Low-speed         Obsolete         High-speed
                switches                           routers
```

---

#### Real World: What High-Speed Routers Do

Modern high-speed routers (Cisco CRS, Juniper T-series) use VOQ with sophisticated schedulers:

```text
┌─────────────────────────────────────────────────────┐
│                                                     │
│  Line Cards (input)    Fabric    Line Cards (output)│
│                                                     │
│  [VOQ→O1]  ┐                    ┌  [output queue]  │
│  [VOQ→O2]  ├──────────────────►─┤  [output queue]  │
│  [VOQ→O3]  ┘   crossbar fabric  └  [output queue]  │
│                                                     │
│  Scheduler runs iSLIP or similar every time slot    │
│  Time slot = one cell time (fixed-size cell)        │
│                                                     │
└─────────────────────────────────────────────────────┘
```

Many high-speed switches also use **cell-based switching** internally — variable-length packets are segmented into fixed-size cells (like ATM 53-byte cells) for switching, then reassembled at the output. Fixed size makes scheduling much simpler and more predictable.

---

#### Summary

```text
Problem: N inputs, N outputs, packets arrive simultaneously
         → contention when multiple packets want same output

OQ:  Queue at output
     + Best delay, 100% throughput, work-conserving
     - Fabric must run at N×R → impractical at high speeds

IQ:  Queue at input (one FIFO per input)
     + Fabric runs at 1×R → practical
     - HOL blocking → only 58% throughput
     - Wasted bandwidth even when outputs are idle

VOQ: Separate queue per input per output (N² queues)
     + Fabric runs at 1×R → practical
     + Eliminates HOL blocking → ~100% throughput
     + Best of both worlds
     - Needs smart O(N²) scheduler (iSLIP etc.)
     - Used in all modern high-speed routers
```

The progression OQ → IQ → VOQ is a beautiful example of **identifying the root cause** (one queue conflates packets for different destinations) and fixing it precisely (separate queues per destination) rather than patching around it.

---

## Section 08 — Rate Guarantees: Giving Each Flow Its Fair Share

### What is rate guarantee ?

A rate guarantee means: 
A flow is guaranteed at least a certain bandwidth.

The key idea is:
The router must distinguish flows and schedule them intelligently.

#### Link Capacity / Link Rate / Line Rate

Suppose you have an Ethernet link: 100 Mbps Ethernet
Then: 
Link Capacity = 100 Mbps
Link Rate = 100 Mbps
Line Rate = 100 Mbps

These all usually mean:
The maximum number of bits that can be transmitted on the physical link per second.



R = link transmission rate in In Packetization (Transmission) Delay
R = link capacity


Flow A has a rate guarantee of 10 Mbps means:
Flow A is guaranteed at least 10 Mbps of service.
Mathematically: Throughput  >= 10 Mbps

### What Problem Are We Solving?

So far we've discussed **how packets get to the right output port** (forwarding) and **how queues build up** (buffering, A(t)/D(t)). Now there's a new question:

> **When multiple packets are sitting in a queue waiting to be transmitted, in what ORDER do we send them?**

This is the **scheduling discipline** — separate from the *drop policy* (which packets to discard when the buffer is full). Scheduling is about **order and rate**, not survival.

```text
Output port queue:
┌──────────────────────────────────────┐
│ [Flow A pkt][Flow B pkt][Flow A pkt] │ ──► link (rate R)
└──────────────────────────────────────┘
                  ↑
          Which one goes next?
          This is the scheduling decision.
```

The simplest answer — **FIFO** — sends them in arrival order. But FIFO has a problem: it provides **no isolation**. A single greedy flow sending as fast as possible can occupy the entire queue and starve everyone else.

```text
FIFO with one greedy flow:

Flow A (greedy): ████████████████████████████████████
Flow B (normal): █░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
                  ↑
            Flow B's packets sit behind Flow A's flood,
            experiencing huge queueing delay,
            even though Flow B itself sends very little.
```

This motivates everything below.

---




### Strict Priority Queueing

Replace the single FIFO queue with two queues: high-priority and low-priority. The server always empties the high-priority queue first; it only serves the low-priority queue when the high-priority queue is completely empty.

- High-priority traffic is completely unaffected by low-priority traffic. 
- The downside: if high-priority traffic is heavy, low-priority traffic may never get served (starvation).

#### Mechanism

Packets are classified into priority classes. The scheduler **always serves the highest-priority non-empty queue first**. A lower-priority queue is served **only when all higher-priority queues are empty**.

```text
Priority 0 (highest): [V1][V2]        ← VoIP/control traffic
Priority 1:           [D1][D2][D3]    ← interactive data
Priority 2 (lowest):  [B1][B2][B3][B4]← bulk transfer

Scheduler always checks top-down:
  Priority 0 non-empty? → send V1
  Priority 0 non-empty? → send V2
  Priority 0 empty, Priority 1 non-empty? → send D1
  ...
```

#### Worked Example

```text
Time:        t=0   t=1   t=2   t=3   t=4   t=5
Pri-0 queue: [V1]  [V2]  []    [V3]  []    []
Pri-1 queue: [D1]  [D1]  [D1]  [D1]  [D2]  []

Transmission order: V1, V2, D1, V3, D2

D1 sat in queue from t=0 to t=3 (3 time slots) — delayed
purely because higher-priority packets kept arriving.
```

#### The Starvation Problem

```text
If Priority 0 traffic NEVER stops arriving:

Priority 0: ████████████████████████████████████ (continuous)
Priority 1: ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ (never served!)

Priority 1 receives ZERO service — complete starvation.
This is by design: strict priority gives NO guarantee to lower classes.
```

#### When Strict Priority Is Appropriate

It works well when the high-priority class is **small in volume** and **latency-sensitive**:

```text
Real router example:
  Priority 0: routing protocol packets (OSPF, BGP hellos)
              — tiny volume, but if delayed, routes flap
  Priority 0: VoIP packets — small, bounded volume,
              extremely latency-sensitive (jitter destroys calls)
  Priority 1: everything else (web, bulk transfer)
```

Because Priority 0 traffic is a *small, bounded* fraction of link capacity, starving Priority 1 in practice doesn't happen — Priority 0 finishes quickly and yields. The danger only materializes if Priority 0 traffic is unbounded (e.g., a DoS attack flooding the high-priority queue).

---

### Rate Guarantees

#### What It Means

A **rate guarantee** is a promise: *"Flow (or class) i will receive at least rate r_i, regardless of what other flows are doing — as long as flow i has data to send."*

This is a **fundamentally different axis** from priority:

```text
Priority   →  controls ORDER / LATENCY
              "who goes first when there's contention"

Rate       →  controls THROUGHPUT / BANDWIDTH SHARE
guarantee     "how much of the link each flow gets over time"

A flow can have:
  - high priority but no rate guarantee (gets served first,
    but if it never sends, gets 0% — that's fine for it)
  - a rate guarantee but low priority (gets, say, 20% of link
    minimum, but its packets aren't served "first")
```

#### Why We Need Rate Guarantees

**1. Fairness / isolation between users.** On a shared access link (e.g., an ISP's edge router serving 100 customers), one customer running a P2P client shouldn't be able to consume 100% of the link and reduce everyone else to near-zero.

```text
Without rate guarantee (FIFO):
  Customer A (heavy):  gets 95% of link
  Customer B (light):  gets 5% of link — starved

With rate guarantee (each gets ≥ 1% minimum):
  Customer A:  capped to fair share when B is active
  Customer B:  guaranteed its 1% even if A floods
```

**2. SLA enforcement.** ISPs sell tiers: "Gold" customers get guaranteed 100 Mbps, "Bronze" customers get guaranteed 10 Mbps, with any unused capacity shared opportunistically.

**3. Protecting real-time flows from greedy flows**, without requiring strict priority (which could starve everything else).

#### The Key Property: Isolation

A good scheduler with rate guarantees provides:

```text
Flow i's achieved rate ≥ r_i,
  regardless of how many other flows are active,
  regardless of how aggressive other flows are,
  as long as flow i itself has traffic to send.

This is sometimes called "protection" or "isolation" —
one flow's misbehavior cannot violate another flow's guarantee.
```

This is precisely what **FIFO cannot provide** — in FIFO, one flow's send rate directly steals bandwidth from others with no floor.

---

### Weighted Fair Queueing (WFQ)
#### The key idea:

WFQ provides rate guarantees by controlling how often each flow gets service from the output link.

#### Separate Queues Per Flow

WFQ creates a queue for each flow.
```
Flow A Queue: [A][A][A]

Flow B Queue: [B][B]

Flow C Queue: [C][C][C][C]


               Output Link
                   |
                   v

A Queue --->

B Queue ---> Scheduler ---> Link

C Queue --->

```

Now the scheduler chooses which queue to serve next.
WFQ chooses packets so that each flow receives service proportional to its weight.
This is the crucial difference.

#### The Goal

WFQ is a scheduling discipline that gives each flow `i` a **weight `φᵢ`**, and guarantees:

```
Flow i's rate ≥ (φᵢ / Σφⱼ) × R     (when flow i is backlogged)

where R = link capacity, sum is over all currently-active flows
```

- If all weights are equal, this is simply **fair sharing**: N active flows each get R/N. 
- If weights differ, flows get bandwidth **proportional to their weight** — this is how you implement "Gold gets 10× the share of Bronze."

Suppose:
Weight(A) = 1
Weight(B) = 2
Weight(C) = 3

WFQ interprets this as:
A gets 1/6 of capacity
B gets 2/6 of capacity
C gets 3/6 of capacity


Suppose all flows are continuously backlogged.

Then WFQ ensures the scheduler keeps serving:  A:B:C = 1:2:3 proportionally.
Over time:
1 packet from A
2 packets from B
3 packets from C
Thus:
A receives 1/6 of service
B receives 2/6 of service
C receives 3/6 of service



#### The Ideal (But Impossible) Model: Bit-by-Bit Round Robin / GPS

To understand WFQ, first understand the **idealized fluid model** it approximates, called **Generalized Processor Sharing (GPS)**.

Imagine the link could serve **one bit at a time**, cycling through all active flows in round-robin, weighted by `φᵢ`:

Impossible physically,But useful mathematically.

WFQ approximates GPS. Most QoS theory starts here.

```text
GPS / Bit-by-bit Round Robin (idealized, NOT physically realizable):

Round 1: 1 bit from Flow A, 1 bit from Flow B
Round 2: 1 bit from Flow A, 1 bit from Flow B
Round 3: 1 bit from Flow A, 1 bit from Flow B
...

If both flows are continuously backlogged, each progresses
at exactly rate R/2 — perfectly fair, instant by instant.
```

This model is **perfectly fair regardless of packet sizes** — because fairness is measured bit-by-bit, not packet-by-packet. But real links can't interleave individual bits between flows; they must transmit **whole packets** without preemption. WFQ's job is to **schedule whole packets in an order that closely tracks what GPS would have produced.**

#### Why Naive Round-Robin (One Packet per Turn) Fails

```text
Naive round-robin: send ONE PACKET from each non-empty queue, in turn.

Flow A sends 1000-byte packets.
Flow B sends 100-byte packets.

Round-robin gives each flow "1 turn" — but turns aren't equal in bytes:

Per round: A sends 1000 bytes, B sends 100 bytes
Bandwidth share: A gets 1000/1100 ≈ 91%,  B gets 100/1100 ≈ 9%

If weights were supposed to be equal (50/50), this is WRONG.
Flow A is unfairly advantaged purely because its packets are bigger.
```

WFQ must account for **packet size** when deciding order.

#### Idea of WFQ: A practical way to do WFQ it

Compute when each packet would finish under GPS, then transmit packets in that finish-time order.


“PRETEND” we serve each packet bit by bit. 
Figure out what time it would finish if we *did* serve bit by bit.
Then serve all the packets in the order they *would* have finished under bit by bit.
Nice simple way to do this – very clever.

#### Define Virtual Rounds
Instead of measuring time in seconds:
0 sec
1 sec
2 sec
...
WFQ measures time in **Round numbers** called **virtual time**.


* Define a ROUND to be the time to visit and serve every queue.
* In each round, serve queue I with w_i bits. 
* Do the following calculation all in terms of the ROUND NUMBER:
When packet k arrives: 
packet k  start time in round:
$$S_k = Max( F_{(k-1)}, now ) $$
packet k  finish time in round:
$$F_k = S_k + L_k/w_k $$
$$F_k = Max( F_{(k-1)}, now ) + L_k/w_k$$

Now serve the packets in order of F_k, the FINISHING ROUND.

$F_k $ : Virtual finish round of packet k.
$S_k $ : Virtual start round of packet k.
$F_{(k-1)}$ : Finish round of the previous packet from the same flow.
now : Current virtual round.
$L_k/w_k$: How many rounds are needed to serve packet k.

Scheduler Decision: pick the one with the lowest Finish round for each flow

#### Example

Flow A: weight = 1
Flow B: weight = 2

Packets:
A1 = 100 bits
A2 = 200 bits
B1 = 100 bits

Assume now = 0

Packet A1: $F_1= 0 + 100/1 = 100 $
Packet A2: $F_2= 100 + 200/1 = 300 $

Packet B1: $F_B= 0 + 100/2 = 50 $

so 
A1 finishes round 100  
A2 finishes round 300
B1 finishes at round 50

Scheduler Decision
Now compare:
B1 finish round = 50
A1 finish round = 100


WFQ sends B1 first, A1 second
because GPS would have completed B1 first.

#### Three nice properties:
1. Finishing times can be determined at the packet arrival time. 
2. The packets are served in order of finishing time.  
This approach is called Weighted Fair Queueing (WFQ) or Packetized Generalized Processor Sharing (PGPS)


GPS:Serve bits continuously
WFQ: Serve whole packets

The only error comes from packetization. A packet may have to wait for one currently-transmitting packet to finish.



Sketch time lines of FINISHING ROUND -> FINISHING TIME.
Sketch on it that the difference, DELTA, between the finishing time when calculated using ROUNDS and the actual finishing time  for EVERY packet in the system.

The famous result:
$$\Delta < L_{max}/R $$  

The difference between ideal GPS and actual WFQ is less than one maximum packet transmission time.

This means that over the long term, the DELTA is amortized, and the rate of each flow is simply: 
$$w_i/(Sum(w_i)) * R$$


---

### Practical Implementation: Why Exact WFQ Is Too Expensive

Computing exact virtual finish times requires tracking `V(t)`, which depends on **which flows are currently backlogged** — this set changes every time a flow's queue empties or a new packet arrives. Naive implementations are **O(N) per event**, and even efficient ones (using a sorted structure / heap of finish times) are **O(log N) per packet**. At 100 Gbps with millions of packets per second, O(log N) per packet is often too slow.

#### Deficit Round Robin (DRR) — The O(1) Practical Approximation

DRR is the workhorse used in real routers and Linux `tc` (traffic control). It approximates WFQ with **constant-time** per-packet overhead.

**Setup:**
- Each flow `i` gets a **quantum `Qᵢ`**, proportional to its weight `φᵢ`
- Each flow has a **deficit counter `DCᵢ`**, initialized to 0

**Algorithm (round-robin over active queues):**

```text
For each active queue, in round-robin order:

    DCᵢ += Qᵢ                          // credit this queue with its quantum

    while (queue not empty) AND (DCᵢ >= size of packet at head):
        send packet
        DCᵢ -= packet_size

    if queue becomes empty:
        DCᵢ = 0                         // reset, don't carry over credit
    else:
        // queue still has packets, but the next one is too big
        // for the remaining DCᵢ — leave DCᵢ as-is, move to next queue
        move to next queue in round-robin
```

#### Worked Example

```text
Quantum Q = 500 bytes for both flows.

Flow A queue: [700][700][700]   (large packets)
Flow B queue: [200][200][200]   (small packets)

Round 1, Flow A:
  DC_A = 0 + 500 = 500
  head packet = 700, DC_A (500) < 700 → CANNOT SEND
  move to Flow B, DC_A stays at 500 (carried over)

Round 1, Flow B:
  DC_B = 0 + 500 = 500
  head packet = 200, DC_B (500) >= 200 → SEND, DC_B = 300
  head packet = 200, DC_B (300) >= 200 → SEND, DC_B = 100
  head packet = 200, DC_B (100) <  200 → STOP
  move to Flow A, DC_B stays at 100

Round 2, Flow A:
  DC_A = 500 + 500 = 1000
  head packet = 700, DC_A (1000) >= 700 → SEND, DC_A = 300
  head packet = 700, DC_A (300) < 700 → STOP
  move to Flow B, DC_A stays at 300

Round 2, Flow B:
  DC_B = 100 + 500 = 600
  head packet = 200, DC_B (600) >= 200 → SEND, DC_B = 400
  head packet = 200, DC_B (400) >= 200 → SEND, DC_B = 200
  (queue now empty) → DC_B = 0
```

```text
Transmission order: B(200) B(200) A(700) B(200) B(200) ...

Over 2 rounds:
  Flow A sent: 700 bytes
  Flow B sent: 600 bytes

Close to the intended ~equal shares (500/500 per round, on average,
with small discrepancies due to packet-size granularity — this
"unfairness" is bounded by at most one maximum packet size, which
is the known accuracy bound of DRR).
```

#### Why DRR Wins in Practice

```text
                    Exact WFQ          DRR
─────────────────────────────────────────────────────
Per-packet cost     O(log N)           O(1)
Fairness accuracy   Exact (ideal)      Within 1 max packet size
Implementation      Complex (heaps,    Simple (counters,
                    virtual time)      round-robin pointer)
Used in             Theoretical /      Linux tc, hardware
                    high-end research  switches/routers
                    routers            (very common)
```

DRR's bounded unfairness (at most one maximum-sized packet's worth of discrepancy per round) is acceptable in practice, and the O(1) cost makes it feasible at line rate.

---

### Combining Everything: How Real Routers Do It

Production routers typically use a **hierarchical scheduler** combining strict priority and WFQ/DRR:

```text
┌─────────────────────────────────────────────────────────┐
│                                                         │
│   Priority Queue 0 (strict priority, small bandwidth   │
│   cap): VoIP, routing protocol control traffic         │
│         ─────────────────────────────────              │
│                      │ served first, but bandwidth-     │
│                      │ limited (policed) so it can't     │
│                      │ starve everything else            │
│                      ▼                                   │
│   ┌─────────────────────────────────────────┐          │
│   │  WFQ / DRR among remaining classes:      │          │
│   │  Gold customers   (weight 10)            │          │
│   │  Silver customers (weight 5)             │          │
│   │  Bronze customers (weight 1)             │          │
│   └─────────────────────────────────────────┘          │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

Cisco calls this **Low Latency Queueing (LLQ)**: one strict-priority class (bandwidth-policed so it can never exceed its allocation) sits on top of a WFQ/CBWFQ (Class-Based WFQ) scheduler for everything else. This gives you:

- **Latency guarantee** for real-time traffic (via priority)
- **Throughput/fairness guarantees** for everyone else (via WFQ)
- **No starvation**, because the priority class is capped

---

### Summary Table

```text
                  Strict Priority      Rate Guarantee /    WFQ
                                       Isolation
──────────────────────────────────────────────────────────────────
Controls          Order / latency      Throughput share     Both
What it gives     "go first"           "get at least X%     Proportional
                                       of the link"          share by weight
Risk              Starvation of        N/A by itself —      Bounded
                  lower classes        depends on            unfairness
                                       mechanism (e.g. WFQ)  (1 max packet)
Best for          Small-volume,        Multi-tenant         General-purpose
                  latency-critical     isolation, SLAs       fair sharing
                  traffic (VoIP,
                  routing protocols)
Practical impl.   FIFO per class,      Implemented via      Deficit Round
                  served top-down      WFQ/DRR              Robin (DRR)
```

The big-picture takeaway: **priority answers "who's next," rate guarantees answer "how much over time," and WFQ/DRR is the mechanism that delivers rate guarantees in a packet-by-packet system efficiently.** Real routers layer all three together — a small strict-priority class for latency-critical control/voice traffic, sitting on top of a DRR/WFQ scheduler that fairly divides the remaining bandwidth among traffic classes or customers.

In a plain FIFO queue, every packet competes equally for the output link. There's no way to promise any particular flow a minimum rate.



Weighted Fair Queueing (WFQ):
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

This approach is also called Packetized Generalized Processor Sharing (PGPS). Over the long run, WFQ guarantees each flow exactly its weighted share of the link.

> **Key Takeaway:** A plain FIFO queue gives bandwidth to whoever sends the most. Weighted Fair Queueing assigns each flow a weight and guarantees it a proportional share of the link. By computing virtual finish times for variable-length packets, WFQ achieves fair, guaranteed rates without requiring all packets to be the same size.

---

## Section 09 — End-to-End Delay Guarantees

A delay guarantee means: A packet will be delivered within a bounded time.

The fundamental question is: How can a packet-switched network provide a deterministic upper bound on end-to-end
The answer is: We must control both the input traffic and the service provided by routers.

### Bounded Delay in a Single Router

A flow served by a WFQ queue with rate rᵢ and a buffer of size B bits waits at most B/rᵢ seconds before being served. 
$$D_q = B/r_i$$

What we already know how to control the delay of packets?
1. The size of each queue.
2. The serivce rate at which a queue is serverd(WFQ).

If queue length is bounded, and service rate is guaranteed, then delay is bounded.

If we know the maximum queueing delay at each router along the path, we can add them up to bound the end-to-end delay.




### How to bound Queue length?
### Contraining traffic

Number of bits that can arrive in any period of length T is bounded by $\sigma + \rho T$

This is called a **(σ, ρ) regulation**. 

```
Traffic constraint:   A(t + T) − A(t) ≤ σ + ρ·T    for all t, T ≥ 0

σ = Maximum burst size (bits)
ρ = Long-term average rate (bits/second)


During any interval of length T, at most σ + ρT bits may arrive.
```
It allows short bursts up to B bits but limits the long-term average rate to ρ.

when $\sigma=B, \rho=R_i$


####  The Leaky Bucket: Traffic Shaping

To prevent buffer overflow, a **leaky bucket** (token bucket) constrains the traffic generated by a source. 
In any time interval T, the source may send at most  σ  + ρ·T bits:

#### The Leakly bucket regulator

Token bucket parameters:
- bucket size = σ
- token generation rate = ρ

Tokens accumulate. Packet can only leave if enough tokens exist.
Thus token bucket exactly enforces  σ  + ρ·T 


#### The Leaky Bucket + WFQ guarantees service

Traffic entering router:
- Burst ≤ σ
- Average rate ≤ ρ

The source shapes its traffic with parameters (B, ρ). 
Each router puts the flow into a WFQ queue served at rate rᵢ with buffer size B. 
Because the arrival process is (B, ρ)-constrained and ρ <= rᵢ , Arrivals occur slower than service.
the queue never overflows.

Maximum backlog becomes finite.
Maximum queue occupancy: Queue length bounded by approximately
$$B_{max} = \sigma$$

Maximum Delay: The queue delay bounded by 
$$D_i = \sigma/r_i$$

#### Single router picture
```
Source
 │
 │  (σ,ρ)
 ▼

+----------------+
|    Queue       |
+----------------+

service = r_i

▼

Output
```



### End-to-End Delay

If flows are **leaky bucket** constrained, and routers use WFQ, then end-to-end delay guarantees are possible.

```
max_delay = Σᵢ (packetization_delay_i) + Σᵢ (propagation_delay_i) + Σᵢ (B / rᵢ)
```
#### RSVP

This guarantee is coordinated by **RSVP** (Resource Reservation Protocol, IETF RFC 2205), which signals the required rates and buffer sizes to every router along the path before the flow begins.


RSVP allows applications to request resources.
- Sender says
```
I need

ρ = 2 Mbps

σ = 50 KB

delay < 30 ms
```

- Each router checks
```
Do I have enough bandwidth?

Enough buffer?

Enough scheduler capacity?
```

If yes, reserve resources. Otherwise, reject.


#### The Big Picture
End-to-end delay guarantees work because every component that can create delay is explicitly bounded.

```
Source
 │
 │
 │  Token Bucket
 │
 ▼

(σ,ρ)-regulated traffic


Router 1
WFQ rate r₁
Queue ≤ B₁
Delay ≤ B₁/r₁


Router 2
WFQ rate r₂
Queue ≤ B₂
Delay ≤ B₂/r₂


Router 3
WFQ rate r₃
Queue ≤ B₃
Delay ≤ B₃/r₃


Destination
```

The fundamental theorem behind deterministic QoS is:

If traffic is (σ,ρ)-constrained, every router provides a guaranteed service rate rᵢ > ρ, and admission control ensures enough resources are available, then queue sizes remain bounded, queueing delays remain bounded, and the end-to-end delay is simply the sum of per-hop delay bounds.

This combination of Token Bucket + WFQ + RSVP/Admission Control is essentially the classical Integrated Services (IntServ) architecture proposed in the 1990s.



#### An Example

In the network below, an application wans a rate of 10 Mb/s and an end to end delay of less than 5 ms for 1000 Bytes package

```
   10km, 1Gb/s             100km, 100Mb/s                 10km, 1Gb/s  
A -------------> router_1 ------------------> router2 ----------------> B

Packetization Delay = 2(1000*8/10^9) + 1000*8/10^8 
Propagation Delay   = 120*10^3/2*10^8
```

Fixed delay is:  
Packetization delay + propagaton delay = 0.696ms
Therefore queueing delay needs to be less than 5-.696 = 4.304ms

Let’s CHOOSE to split the delay equally among the two routers: 
i.e. 2.152ms at each.  

Therefore,  
$D_i = \sigma/r_i $ --->  $\sigma = D_i * r_i$
$B > \sigma = 10Mb/s * 2.152ms = 21520 bits = 2690 bytes  $



A needs to send with leaky bucket parameters 21520 bits and  10Mb/s.







### Worked Example


Two hosts are separated by three routers and four 250 km links running at 100 Mb/s. They send 1,500-byte packets at 15 Mb/s and need end-to-end delay under 10 ms. 
if each router serves the flow at 15Mb/s, how long will packets be delayed in each router?

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
queue_delay = max_e2e_delay - fixed_delay           # = 4.52 ms
per_router_delay   = queue_delay / n_routers        # = 1.507 ms each

# Buffer size needed at each router
buffer_bits  = flow_rate * per_router_delay          # ≈ 22,605 bits
buffer_bytes = buffer_bits / 8                       # ≈ 2,826 bytes
# Round up to 2 packets = 3,000 bytes
```

*The fixed delay consumes 5.48 ms of the 10 ms budget. The remaining 4.52 ms is split equally among the three routers — about 1.5 ms per router, requiring roughly 2,826 bytes of buffer per router.*

> **Key Takeaway:** End-to-end delay guarantees require two things working together: traffic shaping at the source (leaky bucket) to prevent buffer overflow, and WFQ scheduling at each router to ensure each flow gets its reserved rate. With both in place, every router's queueing delay is bounded, and the sum of all delays along the path can be guaranteed never to exceed a target.


### Bonus: Delay Bounds (Parekh-Gallager)

If a flow's traffic is **shaped** by a token bucket (bounded burst size `b`, sustained rate `r` ≤ its WFQ-guaranteed rate), WFQ provides a **provable upper bound on end-to-end queueing delay** across multiple hops. This is the theoretical foundation for **Integrated Services (IntServ)** and guaranteed-bandwidth QoS — a flow that respects its contract is mathematically guaranteed bounded latency, no matter what other flows do.


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


# QA


Great questions. These are two completely separate confusions, let me address both precisely.

---

## Question 1: What Is `eth0`? Is It the Same as Port 8000?


### `eth0` — A Network Interface (Physical Port)



```text
A router physically looks like this:

                    ┌─────────────────────────┐
    Cable from ─────┤ eth0  │ eth1  │ eth2    │
    upstream ISP    │       │       │         │
    Cable to ───────┤       │       │         │
    your LAN        └─────────────────────────┘
                      ↑       ↑       ↑
                 physical  physical  physical
                   port      port     port
                 (NIC)      (NIC)    (NIC)
```

`eth0` is the **Linux name for a physical network interface card (NIC)** on the router. It refers to a hardware port where you plug in an Ethernet cable.
Each `eth0`, `eth1`, `eth2` is a **separate physical interface** with its own MAC address and possibly its own IP address. This is a **Layer 1/2 concept** — actual hardware.

Modern Linux names them differently (like `enp3s0`, `ens33`) but the idea is the same.

### Port 8000 — A TCP/UDP Port Number

Port 8000 is a **software concept at the transport layer (Layer 4)**. It is a 16-bit number in the TCP or UDP header that identifies which **application process** should receive the data.

```text
Port 8000 lives here:

┌─────────────────────────────┐
│ Application (web server)    │  ← listens on port 8000
├─────────────────────────────┤
│ TCP header: dst_port=8000   │  ← Layer 4
├─────────────────────────────┤
│ IP header: dst=203.0.113.1  │  ← Layer 3
├─────────────────────────────┤
│ Ethernet: DA=AA:BB:CC:..    │  ← Layer 2
├─────────────────────────────┤
│ Physical cable (eth0)       │  ← Layer 1/2
└─────────────────────────────┘
```

### Side-by-Side Comparison

```text
                eth0                    port 8000
─────────────────────────────────────────────────────
What is it?   Physical NIC            Software number
Layer         Layer 1/2               Layer 4 (TCP/UDP)
Lives where?  Hardware                TCP/UDP header
Identified by Linux interface name    16-bit integer
Purpose       Send/receive frames     Identify application
Has MAC?      Yes                     No
Has IP?       Yes (configured)        No
Router uses?  Yes (egress/ingress)    No (routers don't care)
```

A router forwards packets based on **destination IP** (Layer 3) and sends them out a physical interface like `eth0` (Layer 1/2). It **never looks at TCP port numbers** during normal forwarding — that is a Layer 4 concern for the endpoint application.


Great question. This connects the **Linux software naming** to the **physical hardware** you actually see on a router.

---

### Physical Reality: What You Actually See on a Router

#### Home Router (Consumer Device)

```text
Back panel of a typical home router:

┌─────────────────────────────────────────────────────────┐
│                                                         │
│  [WAN/Internet]  [LAN1] [LAN2] [LAN3] [LAN4]  [USB]   │
│                                                         │
└─────────────────────────────────────────────────────────┘
       ↑                ↑─────────────↑
  1 port for ISP        4 ports for your devices
  (upstream)            (downstream)
```

#### ISP/Enterprise Router (Cisco, Juniper)

```text
Front panel of enterprise router:

┌─────────────────────────────────────────────────────────┐
│                                                         │
│  [GE0/0] [GE0/1] [GE0/2] [Serial0] [Console] [Mgmt]   │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

### The Naming Is Just a Label — Same Underlying Concept

All of these names — `eth0`, `LAN1`, `WAN`, `GE0/0`, `COM` — refer to the **same fundamental thing**:

```text
A physical network interface = one plug on the router
                             = one NIC
                             = one MAC address
                             = one point to send/receive frames
```

Different vendors and operating systems just **name them differently**:

```text
┌────────────────┬──────────────────────────────────────────┐
│ Name           │ Who Uses It                              │
├────────────────┼──────────────────────────────────────────┤
│ eth0, eth1     │ Linux kernel (traditional naming)        │
│ enp3s0, ens33  │ Linux kernel (modern predictable naming) │
│ LAN1..LAN4     │ Home router web UI (TP-Link, ASUS etc)   │
│ WAN            │ Home router web UI (the ISP-facing port) │
│ GE0/0, GE0/1   │ Cisco IOS (GigabitEthernet slot/port)   │
│ em0, em1       │ FreeBSD                                  │
│ COM            │ Serial/console port (NOT Ethernet)       │
│ Serial0        │ Cisco serial WAN interface               │
└────────────────┴──────────────────────────────────────────┘
```

---

### Mapping: Home Router UI vs Linux Internals

Inside a home router (which runs **embedded Linux**), the relationship looks like this:

```text
What you see in          What Linux sees
web UI (192.168.1.1)     inside the router
─────────────────────────────────────────────────

WAN port          ←───► eth0  (or eth1, depends on hardware)
                         IP: assigned by ISP (e.g. 203.0.113.5)
                         MAC: router's public MAC
                         faces the INTERNET

LAN1 port         ←───► 
LAN2 port         ←───►  eth1  (often all 4 LAN ports are
LAN3 port         ←───►        one interface via internal switch chip)
LAN4 port         ←───► 
                         IP: 192.168.1.1
                         MAC: router's LAN MAC
                         faces YOUR DEVICES

WiFi 2.4GHz       ←───► wlan0
WiFi 5GHz         ←───► wlan1
```

The 4 LAN ports often share **one Linux interface** (`eth1`) because they are connected through a built-in **hardware switch chip** — they behave as one logical interface to Linux even though physically there are 4 sockets.

---

### What Is COM? — Completely Different

`COM` is a **serial console port** — completely different from Ethernet:

```text
COM / Console port:

Purpose:   management and debugging access to the router
Protocol:  RS-232 serial (not Ethernet, not IP)
Use case:  when the router is misconfigured or won't boot
           you plug in a serial cable from your laptop
           get a text terminal directly into the OS

┌──────────────┐     RS-232 serial cable     ┌──────────────┐
│    Router    ├─────────────────────────────┤    Laptop    │
│   [CONSOLE]  │                             │  (minicom /  │
│              │                             │   PuTTY)     │
└──────────────┘                             └──────────────┘

NOT used for forwarding packets.
Only used by network administrators for direct management.
```

On Linux this appears as `/dev/ttyS0` or `/dev/ttyUSB0`, not as a network interface at all.

---

### The Full Unified Picture

```text
Physical Hardware          Linux Name        Role
──────────────────────────────────────────────────────────────
[WAN port]        ←───►   eth0              Faces ISP
                           IP: 203.0.113.5   (upstream)
                           MAC: AA:BB:CC:11

[LAN1 port]       ←───►                     Faces your
[LAN2 port]       ←───►   eth1              devices
[LAN3 port]       ←───►   IP: 192.168.1.1   (downstream)
[LAN4 port]       ←───►   MAC: AA:BB:CC:22

[WiFi antenna]    ←───►   wlan0             Wireless
                           IP: 192.168.1.1   (same LAN subnet)
                           MAC: AA:BB:CC:33

[Console/COM]     ←───►   /dev/ttyS0        Management only
                           NO IP address     NOT for packets
                           NOT a NIC
```

---

## Why Does This Matter for Routing?

Each interface sits on a **different network segment** and gets a **different IP**:

```text
eth0 (WAN):  IP = 203.0.113.5  ← ISP gave this IP
             gateway = 203.0.113.1 (ISP's router)

eth1 (LAN):  IP = 192.168.1.1  ← router assigned this itself
             this IS the gateway for all your devices

Routing table inside home router:

┌──────────────────┬───────────────┬────────┐
│ Prefix           │ Next Hop      │ Port   │
├──────────────────┼───────────────┼────────┤
│ 192.168.1.0/24   │ directly conn │ eth1   │ ← LAN devices
│ 0.0.0.0/0        │ 203.0.113.1   │ eth0   │ ← everything else→ISP
└──────────────────┴───────────────┴────────┘

Packet from laptop (192.168.1.5) to 8.8.8.8:
  arrives on eth1 (LAN port)
  lookup 8.8.8.8 → default route → eth0 (WAN port)
  send out eth0 toward ISP
```

---

## Summary

```text
eth0, eth1    = Linux kernel names for physical Ethernet interfaces
LAN1..LAN4    = home router UI names for the same physical ports
WAN           = home router UI name for the ISP-facing port
COM/Console   = serial management port, NOT an Ethernet interface,
                NOT used for packet forwarding at all

They are all just names for physical ports on the router.
Different vendors name the same hardware differently.
Inside Linux, every Ethernet port is eth* (or enp*).
Inside a Cisco router, every port is GE*/FastEthernet*/Serial*.
The concept is identical: one plug = one interface = one MAC address.
```


---

