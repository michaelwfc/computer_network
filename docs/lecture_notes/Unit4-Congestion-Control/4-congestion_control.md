# TCP Congestion Control: A Complete Tutorial

> Based on Stanford CS144 — Unit 4: Congestion Control

---

## Table of Contents

1. [What Is Congestion?](#1-what-is-congestion)
2. [Why Congestion Happens](#2-why-congestion-happens)
3. [Fairness and Max-Min Fairness](#3-fairness-and-max-min-fairness)
4. [Where to Put Congestion Control?](#4-where-to-put-congestion-control)
5. [AIMD: The Core Algorithm](#5-aimd-the-core-algorithm)
6. [AIMD with a Single Flow](#6-aimd-with-a-single-flow)
7. [AIMD with Multiple Flows and the Throughput Equation](#7-aimd-with-multiple-flows-and-the-throughput-equation)
8. [TCP History: The Collapse](#8-tcp-history-the-collapse)
9. [TCP Congestion Window and Slow Start](#9-tcp-congestion-window-and-slow-start)
10. [TCP Tahoe FSM](#10-tcp-tahoe-fsm)
11. [RTT Estimation](#11-rtt-estimation)
12. [Self-Clocking](#12-self-clocking)
13. [TCP Reno: Fast Retransmit and Fast Recovery](#13-tcp-reno-fast-retransmit-and-fast-recovery)
14. [Why AIMD Converges to Fairness: The Chiu-Jain Plot](#14-why-aimd-converges-to-fairness-the-chiu-jain-plot)
15. [Worked Example: AIMD Sawtooth Calculations](#15-worked-example-aimd-sawtooth-calculations)
16. [Summary](#16-summary)

---

## 1. What Is Congestion?

**Intuition:** Congestion happens when more data arrives at a router than it can forward. Like a highway on-ramp during rush hour — cars pile up because the highway can't absorb them fast enough.

### Time scales of congestion

Congestion is not a single event. It occurs across a wide range of time scales:

| Time scale | Example |
|---|---|
| ~100 µs | Two packets arriving simultaneously at a router |
| ~10 ms | Multiple flows saturating a link |
| ~hours | Flash crowds (too many users on a peak-hour link) |

### What happens inside a router

```
                   ┌──────────────┐
  In1 (10 Mb/s) ──►│              │
                   │  Router      ├──► Out (10 Mb/s, bottleneck)
  In2 (10 Mb/s) ──►│  + Queue     │
                   └──────────────┘

  If In1 + In2 > Out capacity:
    → queue fills
    → if queue overflows: PACKET DROP
```

**Key insight:** Congestion is unavoidable — and arguably *desirable*:

- Empty queues = low delay, but **wasted capacity**
- Occupied queues = higher delay, but **efficient usage**

The goal is to keep queues occupied without overflowing them.

---



## 2. Why Congestion Happens

Consider two senders A and B, both sending at 10 Mb/s toward a bottleneck link of 10 Mb/s total:

```
  A (10 Mb/s) ─────────────►
                              ──► bottleneck (10 Mb/s)
  B (10 Mb/s) ─────────────►
```

Combined arrival rate = 20 Mb/s. The link can only drain at 10 Mb/s. Queue grows. If the buffer is finite, packets are dropped.

**Observations:**

1. Congestion is inevitable in a shared packet-switched network.
2. When packets are dropped, retransmissions add *more* load — making congestion worse.
3. Dropped packets waste all the resources used on their upstream path.
4. We need a definition of fairness to share bottleneck links.

---


### Q1: Are there any relations between packet switching and congestion control?

Yes — they're deeply connected. Congestion control exists *because* of how packet switching works. Let me trace the causal chain.

---

#### Packet switching is the root cause of congestion

In **circuit switching**, bandwidth is reserved end-to-end before any data flows. A call from Tokyo to New York gets a dedicated slice of every link along the path. Congestion in the traditional sense simply cannot happen — if no circuit is available, you get a busy signal upfront. The network never overcommits.

**Packet switching** deliberately abandons that guarantee. Every router independently forwards each packet on a best-effort basis, with no prior reservation. Multiple flows compete for the same output link, and routers buffer the excess in a queue. This is exactly what makes packet switching efficient — links are shared, so a link sitting idle for one flow can carry bursts from another.

But that efficiency comes with a price: **queues can fill up, and when they do, packets are dropped.** That's congestion. It's not a bug — it's a structural consequence of the design.

---

#### The statistical multiplexing gamble

Packet switching works on the assumption that flows don't all burst simultaneously. If 10 flows each need 1 Mb/s on average but only burst occasionally, a 5 Mb/s link can serve them all well most of the time. This is **statistical multiplexing**.

Congestion happens when the gamble loses — when too many flows burst at once and demand exceeds the link rate. The queue absorbs the short-term excess (this is good — it prevents packet loss for brief spikes). But if the overload is sustained, the queue overflows, packets drop, and TCP must react.

```
Statistical multiplexing:
  Flows:  ──▓──────▓──────▓──
          ──────▓──────▓─────
          ──▓──────▓──────▓──
  Link:   ──▓▓▓▓──▓▓▓▓──▓▓▓─  (link busy but not overloaded most of the time)

When the gamble fails:
  Flows:  ──▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓──
          ──▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓──
  Queue:  fills → overflows → DROPS
```

Congestion control is how TCP detects that the gamble has failed and backs off.

---

#### The store-and-forward model creates queueing delay

In packet switching, every router **stores the entire packet before forwarding it**. This introduces finite queue occupancy at each hop. The A(t)/D(t) framework you've seen captures this directly:

- `A(t)` = cumulative bits arriving
- `D(t)` = cumulative bits departing
- `Q(t) = A(t) - D(t)` = queue occupancy at any moment

When A(t) persistently exceeds D(t), Q(t) grows without bound — until the buffer overflows. AIMD is TCP's mechanism for keeping A(t) ≤ D(t) on average across the network.

---

#### Packet loss is the congestion signal

In circuit switching there's no equivalent signal — you either have a circuit or you don't. In packet switching, **the drop itself is the signal.** TCP has no direct visibility into router queue depths. It can't ask "how full are your queues?" Instead, it infers congestion from:

- **Timeout** — a packet was likely dropped
- **Triple duplicate ACKs** — a packet was dropped but the pipe isn't fully broken

This is why congestion control lives at the endpoints rather than in the network. The routers in a packet-switched network are stateless with respect to individual flows — they just drop when overwhelmed. The endpoints have to pick up the pieces.

---

#### The bandwidth-delay product ties it together

The amount of data "in flight" in a packet-switched network at any moment is:

```
BDP = bandwidth × RTT
```

This is the size of the "pipe" — how many bytes exist in transit between sender and receiver. TCP's congestion window is precisely an estimate of this value. AIMD probes upward toward the BDP, then backs off when it overshoots and causes a queue to overflow.

The fact that the BDP exists at all — that bytes are in transit simultaneously across many routers, each with their own queue — is a direct consequence of packet switching. In a circuit-switched network, the "pipe" is a reserved resource; there's nothing to probe.

---

In one sentence: **packet switching creates shared queues, shared queues can overflow, congestion control is TCP's distributed algorithm for preventing that overflow — using only the signals observable at the endpoints.**

### Q2: What are the relations between Rate guarantee and Delay guarantee with congestion control ?

Great question — this gets at a fundamental tension in network design. Let me build the intuition carefully.

---

#### Start with what packet switching actually guarantees

The answer is: **nothing.** Best-effort packet switching gives you zero guarantees on rate or delay. A packet might arrive in 1ms or 1 second. It might not arrive at all. Congestion control operates entirely within this best-effort world — it tries to be a *good citizen*, not to obtain guarantees.

So to understand the relationship, you need to contrast two worlds:

```
World A: Best-effort (what the Internet is)
  - No rate guarantee
  - No delay guarantee
  - Congestion control: TCP's AIMD tries to share fairly

World B: Quality of Service (QoS)
  - Rate guarantees possible
  - Delay guarantees possible
  - Congestion control: becomes simpler or unnecessary
```

---

#### Rate Guarantees and Congestion Control

##### What a rate guarantee means

A **rate guarantee** (also called a bandwidth guarantee) promises a flow at least R bits/second regardless of what other flows are doing. The classic mechanism for this is **Weighted Fair Queueing (WFQ)** — which you've already seen as the idealized GPS scheduler behind DRR.

##### How rate guarantees interact with congestion control

If every flow has a rate guarantee, congestion control becomes **unnecessary for that flow.** The scheduler at each router enforces the allocation. No queue overflow, no drops, no need for AIMD.

```
With WFQ rate guarantees:
  Flow A guaranteed 5 Mb/s ──► WFQ scheduler ──► always gets 5 Mb/s
  Flow B guaranteed 3 Mb/s ──► WFQ scheduler ──► always gets 3 Mb/s
  Flow C guaranteed 2 Mb/s ──► WFQ scheduler ──► always gets 2 Mb/s
  Total = 10 Mb/s = link capacity → no overload, no drops

  TCP congestion control: irrelevant, cwnd never needs to shrink
```

But this only works if the **sum of guarantees ≤ link capacity** and traffic actually stays within its allocated rate. If a flow bursts above its guaranteed rate, it's in "best-effort" territory again — subject to drops.

##### The admission control problem

Rate guarantees require **admission control** — the network must refuse new flows when there's no capacity left to guarantee. This is exactly what circuit switching does. In packet switching with QoS, a signaling protocol (like RSVP) asks each router along the path to reserve capacity before the flow starts.

```
RSVP flow setup:
  Sender ──► RSVP PATH ──► Router1 ──► Router2 ──► Receiver
  Sender ◄── RSVP RESV ◄── Router1 ◄── Router2 ◄── Receiver
                              ↑              ↑
                         "I have 5 Mb/s   "Me too"
                          reserved"

  If any router lacks capacity: RESV fails → flow rejected
```

TCP congestion control sidesteps admission control entirely. Any flow can start at any time and AIMD will eventually find a stable operating point — but without any guarantee on what that rate will be.

##### The RTT unfairness problem revisited

Even without guarantees, TCP's AIMD throughput equation reveals a hidden "rate allocation":

```
R = √(3/2) / (RTT × √p)
```

Two flows with different RTTs on the same bottleneck converge to different rates — proportional to 1/RTT. This is not a deliberate rate allocation; it's an accidental one that emerges from AIMD's mechanics. WFQ-based rate guarantees would eliminate this unfairness.

---

#### Delay Guarantees and Congestion Control

This is where the relationship gets deeper and more interesting.

##### What a delay guarantee means

A **delay guarantee** bounds the end-to-end latency a packet will experience. For real-time applications — VoIP, video conferencing, online gaming — a packet arriving 500ms late is effectively lost. You need a worst-case delay bound, not just a good average.

##### Why delay guarantees are harder than rate guarantees

Delay has two components in a packet-switched network:

```
Total delay = propagation delay + queueing delay

Propagation: fixed by physics (speed of light × distance) — unchangeable
Queueing:    variable, depends on queue occupancy — this is what congestion control affects
```

Congestion control can only influence **queueing delay**. And crucially, AIMD *deliberately* fills queues:

```
AIMD sawtooth:
  cwnd grows → queue fills → RTT increases → cwnd halves → queue drains → repeat

  The stable operating point has a non-zero queue!
```

This is the **bufferbloat problem**: AIMD keeps queues persistently occupied, which increases queueing delay for every flow sharing that router — including latency-sensitive flows that have nothing to do with the congested TCP flow.

##### The fundamental tension

```
Rate guarantee → need admission control → limit total load → queues can be bounded
Delay guarantee → need bounded queue depth → need bounded load → same requirement

Congestion control (AIMD) → no admission control → load can exceed capacity
                          → queues fill up → delay is unbounded
```

AIMD's goal is to **maximize throughput** (keep the pipe full). A full pipe means non-empty queues. Non-empty queues mean variable, potentially large queueing delay. **High throughput and low delay are fundamentally at odds.**

##### The bandwidth-delay product makes this concrete

Recall:

```
Optimal buffer size = RTT × C

For a 10 Gb/s link with 50 ms RTT:
  Buffer = 0.05 × 10 × 10⁹ = 500 Mb

If that buffer fills up (which AIMD causes it to do):
  Extra delay = 500 Mb / 10 Gb/s = 50 ms of additional queueing delay
```

A packet that arrives when the buffer is full experiences an extra 50ms of delay — on top of the propagation delay. For a VoIP call, that's the difference between acceptable and unusable.

##### How QoS achieves delay guarantees

The key insight is: **bounded delay requires bounded queue depth, which requires bounded arrival rate.**

```
Leaky bucket + WFQ:

  Flow ──► [token bucket] ──► [WFQ queue]
              ↑
         rate r, burst b

  Token bucket: limits burst to b bytes, sustained rate to r
  WFQ: guarantees service within bounded time

  Result: worst-case delay = b/r + (B - b)/C
          where B = total link buffer, C = link rate
```

This is the **Guaranteed Service** model (RFC 2212). It gives you a provable worst-case delay bound. But it requires every router to enforce traffic shaping — which means per-flow state in routers, which is the scalability problem that motivated best-effort packet switching in the first place.

---

#### Putting it all together

```
┌──────────────────────────────────────────────────────────────────────┐
│               Rate/Delay Guarantees vs. Congestion Control           │
│                                                                      │
│  AIMD Congestion Control (best-effort):                              │
│    ✓ Distributed, scalable, no per-flow router state                 │
│    ✓ Self-regulating, converges to fair sharing                      │
│    ✗ No rate guarantee — throughput = f(RTT, p)                      │
│    ✗ No delay guarantee — queues fill, bufferbloat                   │
│    ✗ RTT-unfair — long-RTT flows systematically disadvantaged        │
│                                                                      │
│  QoS with WFQ + Token Bucket (guaranteed service):                   │
│    ✓ Rate guarantee per flow                                          │
│    ✓ Delay guarantee per flow                                         │
│    ✗ Requires admission control                                       │
│    ✗ Requires per-flow state in every router                          │
│    ✗ Does not scale to the Internet                                   │
│                                                                      │
│  The tradeoff:                                                        │
│    Guarantees ←────────────────────────────→ Scalability             │
│    (circuit switching)              (best-effort packet switching)    │
└──────────────────────────────────────────────────────────────────────┘
```

##### Modern compromise: differentiated services (DiffServ)

Rather than per-flow guarantees (which don't scale) or pure best-effort (which gives no guarantees), the Internet today uses a middle ground: **traffic classes** with different priority levels. A VoIP packet is marked as high-priority; a bulk file transfer is marked as low-priority. Routers serve high-priority queues first.

This gives soft delay preferences — not hard guarantees — but it scales, because routers only track a small number of classes, not individual flows.

TCP congestion control still runs on top of this. AIMD doesn't know or care about DiffServ markings; it just responds to drops. The QoS layer shapes the environment in which TCP operates.

---

The core insight to carry forward: **congestion control is TCP's best-effort approximation of what rate and delay guarantees would give you, without requiring the network to make any promises.** It's a remarkably effective approximation — but it remains an approximation, which is why real-time applications often need additional mechanisms on top of it.



## 3. Fairness and Max-Min Fairness


### Motivating example

Three flows A, B, C share a network. Two possible allocations:

| Allocation | rA | rB | rC | Total |
|---|---|---|---|---|
| #1 | 0.25 | 1.75 | 0.75 | 2.75 |
| #2 | 0.5 | 1.5 | 0.5 | 2.5 |

Allocation #1 has *higher* total throughput, but it's clearly unfair — A gets nearly nothing while B takes almost everything. Allocation #2 is fairer.


Good question — let me make the intuition precise, because the slide presents this without fully explaining the reasoning.

---

#### The network topology matters


```
   A ──rA──►┐     (cap = 2)  (cap = 1) 
            ├──►  link 1 ──► link 2 ──► X
   B ──rB──►┘                 
                               ▲
                               |
                               rC
   C ─────────────────────────►┘
```
AND flows A, B, C all share some bottleneck structure.
The key point: **A and B share link 1. All three share link 2.**

---

#### Why this makes Allocation #1 unfair

Look at what each flow gets:

```
Allocation #1:  rA = 0.25,  rB = 1.75,  rC = 0.75
Allocation #2:  rA = 0.5,   rB = 1.5,   rC = 0.5
```

Apply the max-min fairness definition directly:

> Can you increase one flow's rate without decreasing a flow that already has a **lower** rate?

**Test Allocation #1:**

- rA = 0.25 is the lowest rate.
- Can you increase rA? Yes — link 1 carries rA + rB = 0.25 + 1.75 = 2.0. Link 1 is full. So to increase rA, you must decrease rB.
- But rB = 1.75 > rA = 0.25. Decreasing rB to help rA is *allowed* by max-min fairness — you're decreasing a higher-rate flow to help a lower-rate flow.
- Therefore, **Allocation #1 is NOT max-min fair.** It fails because rA can be increased (by taking from rB which has more).

**Test Allocation #2:**

- rA = 0.5, rC = 0.5 are tied for lowest.
- Can you increase rA? Link 1: rA + rB = 0.5 + 1.5 = 2.0. Full. Must decrease rB.
  - rB = 1.5 > rA = 0.5. So decreasing rB is allowed. rA *could* increase.
  - But wait — link 2 also carries rA + rB + rC = 0.5 + 1.5 + 0.5 = 2.5... actually that depends on the exact topology.

---

#### The real intuition the slide is pointing at

Rather than getting lost in the exact topology, here is what the slide is actually illustrating — the **intuitive** reason, which is simpler:

##### Allocation #1 is unfair because the gap between flows is unjustifiably large

```
Allocation #1:
  rA = 0.25  ◄── extremely low
  rB = 1.75  ◄── extremely high (7× more than A)
  rC = 0.75

  B is taking 7× what A gets, for no structural reason.
  A is being starved.
```

##### Allocation #2 is fairer because the minimum is higher and the gap is smaller

```
Allocation #2:
  rA = 0.5   ◄── doubled compared to #1
  rB = 1.5   ◄── reduced (3× more than A, not 7×)
  rC = 0.5

  The weakest flow (A) got a better deal.
  The total throughput dropped slightly (2.75 → 2.5),
  but A's rate improved significantly.
```

This is exactly what max-min fairness prioritizes: **maximize the minimum, even at the cost of total throughput.**

---

#### The core tradeoff the example is showing

```
                    Total throughput    Minimum rate    Fair?
Allocation #1:           2.75              0.25          No
Allocation #2:           2.50              0.50          Closer

Max-min fairness deliberately sacrifices total throughput
to protect the weakest flow.
```

This is the philosophical point: **efficiency and fairness are not the same thing, and sometimes conflict.** Allocation #1 is more *efficient* (higher total throughput). Allocation #2 is more *fair* (the minimum rate is higher).

The Chiu-Jain plot you saw earlier draws this explicitly — the "efficient" line (`rA + rB = C`) and the "fair" line (`rA = rB`) are two different lines, and the ideal point is their intersection.

Max-min fairness says: when you must choose between them, protect the minimum first.


### Max-Min Fairness

> **Definition:** An allocation is max-min fair if you cannot increase the rate of one flow without decreasing the rate of another flow that already has a *lower* rate.

**Intuition:** Max-min fairness maximizes the minimum rate. It protects the weakest flows. Think of it as ensuring everyone gets at least their equal share before anyone gets extra.

**On a single link with N flows and capacity C:**

```
Max-min fair allocation: each flow gets C/N
```

This is both intuitive and easy to compute on a single link. In a multi-hop network it becomes more complex.

### Goals for congestion control

1. **High throughput** — keep links busy
2. **Max-min fairness** — share fairly
3. **Respond quickly** to changing network conditions
4. **Distributed control** — endpoints decide, not a central authority


### Q1 : What are the relations between Rate guarantee and Delay guarantee with congestion control ?

Great question — these two concepts are often conflated because WFQ is frequently described as "achieving fairness." Let me make the distinction precise.

---

#### They answer different questions

This is the most important thing to understand first:

```
Max-Min Fairness:   WHAT should the allocation be?   (a goal)
WFQ:                HOW do we achieve an allocation?  (a mechanism)
```

Max-min fairness is a **mathematical criterion** for evaluating whether a rate allocation is fair. WFQ is a **scheduling algorithm** that runs inside a router. They operate at completely different levels of abstraction.

Confusing them is like confusing "everyone should get equal food" (a principle) with "a round-robin cafeteria line" (a mechanism). The mechanism may or may not achieve the principle, depending on circumstances.

---

#### Max-Min Fairness: the goal

Recall the definition:

> An allocation is max-min fair if you cannot increase the rate of any flow without decreasing the rate of another flow that already has a **lower or equal** rate.

This is a property of a **static rate allocation vector** `(r₁, r₂, ..., rₙ)`. It says nothing about how to achieve that allocation in real time. It's purely a statement about whether the numbers are fair.

##### How to compute the max-min fair allocation

The algorithm is called **progressive filling**:

```
1. Start all flows at rate 0.
2. Increase all rates simultaneously at the same rate.
3. When a link becomes saturated, freeze the rates of all flows
   that pass through it.
4. Continue increasing the remaining unfrozen flows.
5. Stop when all flows are frozen.

Result: the max-min fair allocation.
```

Example with three flows on two links:

```
Link 1 (cap = 10):  Flow A, Flow B
Link 2 (cap =  8):  Flow B, Flow C

Progressive fill:
  t=0: rA=0, rB=0, rC=0
  Increase all equally...

  At r=4: Link 2 saturates (rB + rC = 8 → 4+4=8). Freeze rB and rC at 4.
  Continue increasing rA only...

  At r=6: Link 1 saturates (rA + rB = 10 → 6+4=10). Freeze rA at 6.

  Result: rA=6, rB=4, rC=4

  Check: Can rA increase? No — Link 1 is full.
         Can rB increase? No — Link 2 is full.
         Can rC increase? Only by decreasing rB (which is lower). → Not allowed.
  → Max-min fair ✓
```

Notice that max-min fairness operates on the **entire network topology simultaneously**. It's a global optimization, not a local per-router decision.

---

#### WFQ: the mechanism

WFQ is a **per-router, per-link scheduling algorithm**. It decides the order in which packets in different queues are transmitted on a single output link.

##### The idealized version: GPS

WFQ approximates **Generalized Processor Sharing (GPS)**, which is the theoretical ideal. GPS imagines that packets are infinitely divisible — a router can serve all flows simultaneously at fractional rates, like fluid flowing through pipes.

```
GPS on a 10 Mb/s link with weights w₁=1, w₂=2, w₃=2 (total weight = 5):

  Flow 1 gets: (1/5) × 10 = 2 Mb/s
  Flow 2 gets: (2/5) × 10 = 4 Mb/s
  Flow 3 gets: (2/5) × 10 = 4 Mb/s

  All flows served simultaneously, bit by bit.
```

But packets are not infinitely divisible — a router must transmit one packet at a time. WFQ approximates GPS by computing the **finish time** each packet would have under GPS, then serving packets in order of increasing finish time.

##### WFQ finish time calculation

```
For packet k of flow i with length Lᵢₖ:

  Fᵢₖ = max(Fᵢ,ₖ₋₁, now) + Lᵢₖ / wᵢ

  where wᵢ is the weight of flow i.

Serve packets in increasing order of Fᵢₖ.
```

This is a **local computation** — each router independently computes finish times based on what arrives at that router. It has no knowledge of other routers.

---

#### The key differences

##### 1. Scope: local vs. global

```
WFQ:          Local — operates on ONE link at ONE router.
              Decides which packet to send next out of this output port.

Max-Min Fair: Global — describes the allocation across the ENTIRE network.
              Takes into account every link and every flow's path.
```

A single WFQ scheduler cannot achieve max-min fairness across a multi-hop network. It can only control what happens at its own output port. Max-min fairness is an emergent property that requires coordination across all routers.

##### 2. What they handle: one bottleneck vs. many bottlenecks

```
Single bottleneck link, N flows with equal weights:

  WFQ result:      each flow gets C/N     ✓
  Max-min result:  each flow gets C/N     ✓
  → They agree here.

Multi-hop network, flows with different paths:

  Flow A: link 1 only        (cap = 10)
  Flow B: link 1 + link 2    (cap = 10, cap = 6)
  Flow C: link 2 only        (cap = 6)

  WFQ on each link independently (equal weights):
    Link 1: rA = 5, rB = 5      (split equally)
    Link 2: rB = 3, rC = 3      (split equally)
    Result: rA=5, rB=3, rC=3

  Max-min fair (progressive fill):
    Fill all to r=3: link 2 saturates (rB+rC=6). Freeze B and C at 3.
    Continue A: link 1 has 10-3=7 left, so rA=7.
    Result: rA=7, rB=3, rC=3

  WFQ on link 1 gives rA=5, rB=5.
  But rB is constrained to 3 by link 2.
  So rB=5 is wasted capacity on link 1 — flow A could have had rA=7.
  WFQ ≠ Max-Min Fair in multi-hop networks.
```

This is the fundamental gap. WFQ doesn't know that flow B is constrained downstream. It naively splits link 1 equally between A and B, leaving A underserved.

##### 3. Weighted vs. equal allocation

```
Max-Min Fairness (standard definition): equal allocation to all flows.
WFQ: weighted allocation — flows with higher weights get proportionally more.
```

WFQ generalizes max-min fairness by allowing **intentional differentiation**. You can give a premium video call 3× the weight of a bulk download. Max-min fairness (in its basic form) doesn't have weights — it's about protecting the minimum.

WFQ with equal weights achieves something called **weighted max-min fairness** on a single link, but the plain max-min fairness definition is a special case of that.

##### 4. Time: static vs. dynamic

```
Max-Min Fairness: describes a static allocation at a point in time.
                  Computed once given a fixed set of flows.

WFQ:              operates dynamically, packet by packet, in real time.
                  Handles flows arriving and departing continuously.
                  Adapts to bursts and varying packet sizes.
```

When a flow goes idle in WFQ, its share is automatically redistributed to active flows. Max-min fairness would need to be recomputed from scratch.

##### 5. Delay properties

```
Max-Min Fairness: says nothing about delay — only about rates.

WFQ:              gives a provable worst-case delay bound per flow.

  Delay bound for flow i under WFQ:
    D ≤ Lᵢ_max / wᵢ + Lᵐᵃˣ / C

  where Lᵢ_max = max packet size of flow i
        Lᵐᵃˣ  = max packet size of any flow
        C      = link rate

  This bound is what makes WFQ useful for QoS delay guarantees
  — something max-min fairness doesn't even address.
```

---

#### Summary table

```
┌────────────────────┬──────────────────────────┬──────────────────────────┐
│                    │    Max-Min Fairness       │          WFQ             │
├────────────────────┼──────────────────────────┼──────────────────────────┤
│ What is it?        │ A mathematical criterion  │ A scheduling algorithm   │
│ Scope              │ Global (entire network)   │ Local (one link)         │
│ Allocation type    │ Equal (or weighted)       │ Weighted                 │
│ Topology-aware?    │ Yes                       │ No                       │
│ Multi-hop correct? │ Yes (by definition)       │ No — gaps at bottlenecks │
│ Delay guarantee?   │ No                        │ Yes (provable bound)     │
│ Dynamic?           │ No (static definition)    │ Yes (packet by packet)   │
│ Implementable?     │ Not directly              │ Yes (approximated by DRR)│
│ Relation to AIMD   │ The goal AIMD approximates│ The mechanism AIMD avoids│
└────────────────────┴──────────────────────────┴──────────────────────────┘
```

---

#### Their relationship to congestion control

Bringing it back to the CS144 context:

- **Max-min fairness** is the *goal* that congestion control aspires to. AIMD was designed specifically because it converges toward max-min fair allocation on a single bottleneck (as the Chiu-Jain plot shows).

- **WFQ** is an alternative *mechanism* — instead of relying on endpoints to self-regulate via AIMD, you put a WFQ scheduler in every router and enforce the allocation directly. If every router ran WFQ, TCP congestion control would become less necessary — the network would enforce fairness itself.

- The Internet chose **AIMD over WFQ** because WFQ requires per-flow state in every router, which doesn't scale. AIMD pushes the complexity to the endpoints and keeps routers simple. The price is that AIMD only approximates max-min fairness — and only on a single bottleneck, and only when RTTs are equal.
---

## 4. Where to Put Congestion Control?

Two broad approaches:

### In the network (router-based)

Routers maintain per-flow queues and use a scheduler like **Fair Queueing (FQ)** to allocate bandwidth fairly. Each flow gets its own queue; the scheduler picks round-robin among non-empty queues.

```
Flow 1 ──► [Queue 1] ─┐
Flow 2 ──► [Queue 2] ──┤ FQ Scheduler ──► Link
Flow 3 ──► [Queue 3] ─┘
```

**Pros:** Precise, guaranteed fairness.
**Cons:** Routers must track per-flow state. Does not scale to billions of flows on the Internet.

### At the end host (endpoint-based)

TCP endpoints infer congestion from observable signals (packet loss, delay) and adjust their sending rate. Routers stay simple — they just drop packets when buffers overflow.

**TCP uses end-host-based congestion control.** This is the design the Internet chose: intelligence at the edges, simplicity in the core.
- Reacts to events observable at the end host (e.g. packet loss). 
- Exploits TCP’s sliding window used for flow control. 
- Tries to figure out how many packets it can safely have outstanding in the network at a /me.    
- Varies window size according to AIMD.    
---

## 5. AIMD: The Core Algorithm

TCP varies the number of outstanding packets in the network by varying the window size: 

```
Window size = min{Advertised window, Congestion Window}  
                      Receiver        Transimitter("cwnd")
```

How do we decide the value for cwnd? 


**AIMD = Additive Increase, Multiplicative Decrease**

This is the steady-state algorithm TCP uses to probe for available bandwidth:

```
If a packet is received OK:    W ← W + 1/W   (increase by 1 MSS per RTT)
If a packet is dropped:        W ← W / 2     (halve the window)
```

where `W` is the congestion window in units of MSS (Maximum Segment Size).

#### Q1：What does "increase by 1 MSS per RTT" mean?

The AIMD rule written as `W ← W + 1/W` looks confusing because it is expressed **per ACK**, not per RTT.

Let me derive why these are the same thing.

##### Per-ACK view

Each time an ACK arrives:

```
W ← W + 1/W    (in units of MSS)
```

##### Per-RTT view

How many ACKs arrive in one RTT? If the window is W (in MSS units), then W packets are in flight, so W ACKs come back per RTT.

Total increase over one RTT:

```
ΔW = W packets × (1/W increase per ACK)
   = W × 1/W
   = 1 MSS per RTT
```

So regardless of how large W is, the window grows by exactly **1 MSS per RTT** in congestion avoidance. That is the additive increase part of AIMD.

```
RTT 1:  W = 10 MSS,  receive 10 ACKs,  each adds 1/10  → total +1  → W = 11
RTT 2:  W = 11 MSS,  receive 11 ACKs,  each adds 1/11  → total +1  → W = 12
RTT 3:  W = 12 MSS,  receive 12 ACKs,  each adds 1/12  → total +1  → W = 13
```

Always +1 MSS per RTT, no matter what W is. That is the linear (additive) growth of the sawtooth.

---



### The AIMD Sawtooth

```
cwnd
  │        /│        /│
  │       / │       / │
  │      /  │      /  │
  │     /   │     /   │
  │    /    │    /    │
  │   /     │   /     │
  │  /   ↑drop  ↑drop
  │ /
  └─────────────────────► time

  Each drop: window halved
  Between drops: window grows linearly (additive)
```

The sawtooth is TCP's stable operating point. The average throughput stays high while the network is protected from sustained overload.

---

## 6. AIMD with a Single Flow

### Sending rate and window size

A sender's throughput `R` depends on the window size `W` and RTT:

```
R = W / RTT
```

There are two regimes:

```
Case 1: R × RTT > W
  The pipe is bigger than the window.
  Sender is window-limited — waiting for ACKs.
  ACKs arrive in a burst, not evenly spaced.

Case 2: R × RTT = W   ← optimal
  The pipe is exactly full.
  ACKs arrive evenly spaced — self-clocking (see §12).
  Sender always has something to send.
```

### Buffer sizing for a single flow

For AIMD to keep the link continuously busy, the router buffer must be at least:

```
Buffer size ≥ RTT × C

where C = link capacity
```

If the buffer is too small, the link goes idle between the drop event and when the window recovers.

### Observations for a single flow

1. Window expands and contracts per AIMD, probing how many bytes the pipe can hold.
2. The sawtooth is the stable operating point.
3. Sending rate is effectively constant if buffers are sized correctly (RTT × C).



### Worked Examle
Alice is streaming a high definition video at 10Mb/s from a remote server in San Francisco. All packets are 250bytes long. She measures the ping time to the server and the minimum time she measures is 5ms. Once the AIMD window reaches steady state, for the rest of the video, the sawtooth oscillates between constant minimum and maximum values. The buffer is perfectly sized so that it is just big enough to never go empty. 

1. What is the smallest value of the AIMD window (in bytes)?

2. What is the largest value of the AIMD window (in bytes)?

3. How big is the packet buffer in the router (in bytes)?

4. After a packet is dropped, how long does it take for the window to reach its maximum value again? 

5. If Alice now tries to watch the same video from a server in Australia (with RTT 250ms), how big should the router buffer be (in bytes) if the network still runs at 10Mb/s?  





6. When the server is in Australia, after a packet is dropped, how long does it take for the window to reach its maximum value again (in seconds)? 




Let me build the physical picture first before calculating anything. That's the key to these problems.

---

#### The Physical Picture

The "perfectly sized buffer" constraint tells you something very specific about when the minimum and maximum RTT occur:

```
State 1: Buffer EMPTY, link FULL  ← minimum RTT
State 2: Buffer FULL,  link FULL  ← maximum RTT

The buffer is "just big enough to never go empty" means:
  - At the trough of the sawtooth, AIMD just drained the buffer to zero
  - At the peak,  AIMD just filled the buffer completely
  - The link is ALWAYS full (10 Mb/s) in steady state
```

Visually:

```
cwnd
 │        peak ──────────────────────────
 │             \  buffer full, link full
 │              \
 │               \  (drop → halve)
 │                \
 │  trough ────────────────────────────
 │                  buffer empty, link full
 └─────────────────────────────────────► time

RTT at trough = propagation only       = 5 ms  (given as minimum ping)
RTT at peak   = propagation + buffer   = ?
```

---

#### What "bytes in flight" means

At any moment, the AIMD window = bytes the sender has sent but not yet ACKed = **bytes currently in the network pipe.**

```
Bytes in pipe = bytes on the link + bytes in the buffer

At trough: bytes on link only  (buffer empty)
At peak:   bytes on link + bytes in buffer  (buffer full)
```

The number of bytes on the link at 10 Mb/s for a given RTT:

```
Bytes in pipe = rate × RTT
```

---

#### Why does Bytes in pipe = rate × RTT?

Think about what RTT means physically. It is the time between the sender transmitting a packet and receiving its ACK back. During that entire duration, the sender keeps pumping data into the network at rate R.

```
Sender transmits pkt 1
│
│  ← sender keeps sending at rate R during this whole time
│
│  RTT later: ACK for pkt 1 arrives
```

So the total bytes the sender injected during one RTT, before hearing back about pkt 1, is exactly:

```
Bytes in flight = R × RTT
```

This is the **bandwidth-delay product** — the volume of the pipe between sender and receiver. A useful physical analogy:

```
Imagine a garden hose:
  Rate R  = how fast water flows (liters/second)
  RTT/2   = length of hose / speed of water  (one-way delay)

  Water currently inside the hose = flow rate × transit time
                                   = R × (RTT/2)  one way
                                   = R × RTT      round trip

The sender cannot know what happened to a packet until RTT has passed.
Everything sent in that window is "in the pipe" — unacknowledged, in transit.
```

And the AIMD window W is defined as exactly the number of bytes the sender is allowed to have unacknowledged at once. So:

```
W = bytes in pipe = R × RTT

This is why window size directly maps to bytes in the network.
```

---


#### Why does the window increase by 2,000 bits per RTT, not 1/W?

These are the same thing — just expressed in different units.

In the Alice problem:

```
MSS = packet size = 250 bytes = 2,000 bits
```

The rule says: window increases by **1 MSS per RTT**.

So in bits:

```
Increase per RTT = 1 MSS = 2,000 bits
```

The `1/W` formula is the per-ACK increment expressed in MSS units. When you multiply it out over a full RTT (as shown above), you get exactly 1 MSS = 2,000 bits. They are two views of the same fact:

```
Per-ACK:  +1/W  (fractional MSS, looks small)
Per-RTT:  +1    (whole MSS, the additive increase)

In Alice's case, 1 MSS = 2,000 bits, so per-RTT increase = 2,000 bits.
```

---


#### Q1: Smallest AIMD window (trough)

At the trough, the buffer is empty. The only bytes in flight are those on the link itself (propagation delay only):

```
Window_min = rate × RTT_min
           = 10 Mb/s × 5 ms
           = 10 × 10⁶ bits/s × 5 × 10⁻³ s
           = 50,000 bits
           = 50,000 / 8 bytes
```

**Answer: 6,250 bytes**

---

#### Q2: Largest AIMD window (peak)

At the peak, the buffer is completely full AND the link is full. The RTT doubles because every packet must wait through the entire buffer before being served.

```
RTT_max = RTT_min + buffer_drain_time
        = 5 ms    + 5 ms              ← buffer holds exactly one RTT_min worth
        = 10 ms

Window_max = rate × RTT_max
           = 10 Mb/s × 10 ms
           = 100,000 bits
           = 100,000 / 8 bytes


50,000 bits are in the flight, 50,000 bits are in the buffer
```

Why does the buffer add exactly 5 ms? Because the buffer is "perfectly sized" — it holds exactly as many bytes as the link carries in one minimum RTT. So draining it takes exactly one minimum RTT's worth of time.

```
Bytes in Window_max:
  6,250 bytes on the link  (propagation)
+ 6,250 bytes in buffer    (queueing)
= 12,500 bytes total
```

**Answer: 12,500 bytes**

---

#### Q3: Buffer size

The buffer holds the difference between peak and trough:

```
Buffer = Window_max - Window_min
       = 12,500 - 6,250
```

Which is exactly the classic result:

```
Buffer = RTT_min × rate = 5 ms × 10 Mb/s = 50,000 bits
```

**Answer: 6,250 bytes**

---

#### Sanity check — the AIMD sawtooth

AIMD halves the window on a drop. Does the peak/trough relationship respect that?

```
Window_max / Window_min = 12,500 / 6,250 = 2

And indeed: trough = peak / 2  ✓  (multiplicative decrease by half)
```

This is not a coincidence — it's what determines the buffer size in the first place. The buffer must be exactly Window_min in size so that:
- Draining from peak to trough (halving) empties the buffer exactly
- Filling from trough to peak (additive increase) refills it exactly

The three answers are self-consistent by construction.



Great set of questions. Let me answer each one carefully from first principles.

---



#### Q4: How long to recover from drop to peak? (San Francisco)

From the previous answers:

```
Window at trough (after drop) = 6,250 bytes = 50,000 bits
Window at peak                = 12,500 bytes = 100,000 bits

Distance to climb = 100,000 - 50,000 = 50,000 bits
```

Each RTT the window grows by 1 MSS = 2,000 bits:

```
RTTs needed = 50,000 bits / 2,000 bits per RTT = 25 RTTs
```

What is the average RTT during recovery? The window is climbing from trough to peak, so the buffer goes from empty to full. RTT goes from 5 ms to 10 ms linearly:

```
Average RTT = (5 ms + 10 ms) / 2 = 7.5 ms

Time = 25 RTTs × 7.5 ms = 187.5 ms
```

**Answer: 187.5 ms**

---

#### Q5: Buffer size for Australia server (RTT = 250 ms)

Same formula as Q3, just with a much larger RTT:

```
Buffer = RTT_min × rate
       = 250 ms × 10 Mb/s
       = 250 × 10⁻³ s × 10 × 10⁶ bits/s
       = 2,500,000 bits
       = 2,500,000 / 8 bytes
       = 312,500 bytes ≈ 298 KB
```

**Answer: ~298 KB**

Notice this is exactly 50× the San Francisco buffer (6,250 bytes), because the RTT is 50× larger (250 ms vs 5 ms). The buffer scales linearly with RTT. This is the real-world engineering cost of serving distant users — routers need much larger buffers to keep the link full.

---

#### Q6: Recovery time for Australia server

```
Window at trough = RTT_min × rate = 250 ms × 10 Mb/s
                 = 2,500,000 bits

Window at peak   = RTT_max × rate = 500 ms × 10 Mb/s
                 = 5,000,000 bits

Distance to climb = 5,000,000 - 2,500,000 = 2,500,000 bits
```

Each RTT the window grows by 1 MSS = 2,000 bits:

```
RTTs needed = 2,500,000 / 2,000 = 1,250 RTTs
```

Average RTT during recovery:

```
Average RTT = (250 ms + 500 ms) / 2 = 375 ms

Time = 1,250 × 375 ms = 468,750 ms ≈ 468 seconds ≈ 8 minutes
```

**Answer: ~468 seconds (~8 minutes)**

##### Why is this so catastrophically slow?

The recovery time scales as RTT²:

```
Recovery time = (Buffer / MSS) × Average RTT
              = (RTT_min × rate / MSS) × (3/2 × RTT_min)
              ∝ RTT_min²
```

San Francisco: RTT = 5 ms   → recovery = 187.5 ms
Australia:     RTT = 250 ms → recovery = 468 s

```
Ratio = (250/5)² = 50² = 2,500×  slower recovery
```

This is the brutal mathematical consequence of the TCP throughput equation `R ∝ 1/(RTT × √p)`. Long-RTT connections are penalized twice — they get less throughput AND take far longer to recover from any loss. It is one of the most important practical limitations of AIMD.

---

## 7. AIMD with Multiple Flows and the Throughput Equation

### TCP Throughput Equation

With multiple flows, buffer occupancy fluctuates as each flow has its own sawtooth. The bottleneck buffer is shared and always has packets from many flows.

- For a single flow case:
The RTT changed in lockstep with the AIMD sawtooth
```
Throughput = W(t) / RTT(t) 
```


- Unlike a single flow with multiple flows: 
RTT is essentially **constant** when there are multiple flows 

```
Throughput = W(t) / RTT    ∝ W(t)

The average throughput is the average window size divided by constant RTT.
```

### Simple geometric intuition

From geometric analysis of the AIMD sawtooth:
- Package drop rate:
$$ p= 1/A , where  A= \frac{3}{8}W^2_{max} $$

- Throughput:
$$ R= \frac{A}{W_{max}/2 * RTT} = \sqrt{\frac{3}{2}}\frac{1}{RTT\sqrt{p}}
$$
where:
  R   = throughput of a TCP flow
  RTT = round-trip time
  p   = packet drop rate (probability of loss)


**Interpretation:**

- If RTT → 0, then R → ∞ (but you're physically limited by the link).
- If p → 0, then R → ∞ (but then you're limited by the flow/congestion window).
- **Throughput is inversely proportional to RTT.** Flows with longer RTTs get less throughput — AIMD penalizes high-latency connections.
- **Throughput falls as drop rate rises.** A drop rate of 1% is very different from 0.01%.

### Observations for multiple flows

1. AIMD windows expand/contract per flow, probing the shared pipe.
2. Bottleneck router holds packets from many flows simultaneously.
3. AIMD is very sensitive to loss rate.
4. AIMD penalizes flows with long RTTs (a known fairness issue).

---

## 8. TCP History: The Collapse

### The timeline

| Year | Event |
|---|---|
| 1974 | 3-way handshake developed |
| 1978 | TCP and IP split into TCP/IP |
| 1983 | ARPAnet switches to TCP/IP (January 1) |
| 1986 | **Internet begins to suffer congestion collapse** |
| 1987–88 | Van Jacobson fixes TCP → **TCP Tahoe** |
| 1990 | Fast recovery added → **TCP Reno** |


### TCP Pre-Tahoe
- Endpoint has the flow control window size
- On connection establishment, send a full window of packets
- Start a retransmit timer for each packet
- Problem: what if window is much larger than what network can support?


### What happened in 1986?

Pre-Tahoe TCP had no congestion window. When a connection established, it would **immediately send a full flow-control window of segments** — regardless of network capacity.

If the bottleneck could only queue a few packets, most of the burst would be dropped. TCP would then retransmit them — filling the network with more useless copies. This caused:

> **Congestion Collapse:** The network was working furiously hard sending retransmitted segments that no longer mattered, while applications saw near-zero useful throughput.

Van Jacobson diagnosed the problem and introduced three fixes that are still in use today.


---

## 9. TCP Tahoe


TCP Tahoe introduced three improvements:

1. **Congestion window**
2. **RTT estimation**
3. **Self-clocking** 

### The congestion window (`cwnd`)

- Flow control window is only about endpoint
- Have TCP estimate a congestion window for the network
- Sender window = min(flow control window, congestion window)
- Separate congestion control into two states
  - Slow start: on connection startup or packet timeout, in which it does not follow an AIMD policy.
  - Congestion avoidance: steady operation, in which if follows an AIMD policy.


Flow control limits what the *receiver* can handle. The congestion window limits what the *network* can handle:
The sender never sends more data than either the receiver or the network can absorb.




### Slow Start

- Start with Maximum Segment Size (MSS) 
  `cwnd = 1 MSS(Maximum Segment Size)`
- Increase window by MSS for each acknowledged packet : 
  `cwnd += 1 MSS`
- Exponentially grow congestion window to sense network capacity
  Result: **exponential growth** (window doubles each RTT)
  “Slow” compared to prior approach

```

RTT 1: send 1 segment  → receive 1 ACK  → cwnd = 2
RTT 2: send 2 segments → receive 2 ACKs → cwnd = 4
RTT 3: send 4 segments → receive 4 ACKs → cwnd = 8
...
```

Why is it called "slow"? Because compared to the original approach of sending the *entire flow window immediately*, starting with 1 MSS is slow.



### Congestion Avoidance

- For each ACK: `cwnd += MSS² / cwnd`
- Result: **linear growth** — increases by 1 MSS per RTT
- This is the **additive increase** part of AIMD

### Q1: why how many ACKs arrive per RTT  using cwnd = W × MSS (W packets in flight)?

#### The key insight: how many ACKs arrive per RTT?

If `cwnd` allows W packets in flight, then the sender has W packets outstanding at any moment. When those W packets are delivered, the receiver sends back **W ACKs** — all within one RTT.

```
cwnd = W × MSS   (W packets in flight)

During one RTT:
  sender sends W packets
  receiver delivers W packets
  receiver sends back W ACKs
  sender receives W ACKs
```

---

Great question. Let me build this from absolute basics.

#### What MSS means

**MSS = Maximum Segment Size.**

It is the largest chunk of application data TCP is allowed to put into a single packet. It is negotiated during the TCP handshake and depends on the network path's MTU (Maximum Transmission Unit).

```
Typical values:
  Ethernet:  MSS = 1,460 bytes  (1,500 byte MTU - 20 IP header - 20 TCP header)
  Loopback:  MSS = 65,495 bytes
  CS144:     often simplified to MSS = 1,000 or 1,500 bytes in examples
```

In the Alice example we have been working through:

```
Packet size = 250 bytes = MSS
(the problem states all packets are 250 bytes)
```

MSS is the **unit of measurement** for the congestion window. When we say `cwnd = 4 MSS`, we mean the sender is allowed to have 4 × 1 packet-worth of data unacknowledged at once.

---

#### What W means

W is just a convenient name for the window size expressed **in packets** rather than bytes.

```
cwnd  = total bytes the sender can have in flight
MSS   = bytes per packet
W     = cwnd / MSS = number of packets in flight

Example:
  cwnd = 4,000 bytes
  MSS  = 1,000 bytes
  W    = 4,000 / 1,000 = 4 packets
```

So `cwnd = W × MSS` is just saying:

```
total bytes in flight = (number of packets) × (bytes per packet)
```

Nothing more than that. W is not a special TCP concept — it is just arithmetic for convenience.

---

#### Why W packets in flight means W ACKs per RTT

This is the core of your question. Let me trace exactly what happens during one RTT.

##### What "in flight" means

At any moment, the sender has W packets that have been sent but not yet acknowledged:

```
Time = 0, cwnd = 4 MSS (W = 4):

Sender sends:
  ──► pkt 1 ──────────────────────────────────────────► Receiver
  ──► pkt 2 ──────────────────────────────────────────► Receiver
  ──► pkt 3 ──────────────────────────────────────────► Receiver
  ──► pkt 4 ──────────────────────────────────────────► Receiver

All 4 packets are now "in flight" — sent, not yet ACKed.
Sender STOPS here. cwnd = 4, already 4 packets in flight.
Cannot send pkt 5 until an ACK arrives.
```

##### What happens during one RTT

The receiver gets pkt 1, pkt 2, pkt 3, pkt 4 — and sends back one ACK for each:

```
Sender                              Receiver
  │──── pkt 1 ────────────────────►│
  │──── pkt 2 ────────────────────►│
  │──── pkt 3 ────────────────────►│
  │──── pkt 4 ────────────────────►│
  │                                 │──── ACK 1 ──────────────────►│  ←─┐
  │◄──── ACK 1 ─────────────────── │                               │    │
  │──── pkt 5 ────────────────────►│                               │    │  one RTT
  │◄──── ACK 2 ─────────────────── │                               │    │
  │──── pkt 6 ────────────────────►│                               │    │
  │◄──── ACK 3 ─────────────────── │                               │    │
  │──── pkt 7 ────────────────────►│                               │    │
  │◄──── ACK 4 ─────────────────── │                               │  ←─┘
  │──── pkt 8 ────────────────────►│

W = 4 packets sent → W = 4 ACKs received within one RTT
```

So if W packets are in flight, exactly W ACKs come back within one RTT. This is because:

```
1 packet sent → 1 ACK received (TCP ACKs every packet)
W packets sent in one RTT → W ACKs received in one RTT
```

It is a one-to-one relationship. Every packet that goes out generates exactly one ACK coming back.

---

#### Tying it together with a concrete example

Let MSS = 1,000 bytes, cwnd = 4,000 bytes:

```
W = cwnd / MSS = 4,000 / 1,000 = 4 packets

During one RTT:
  4 packets go out
  4 ACKs come back

Each ACK triggers cwnd update.
That update fires exactly 4 times per RTT.
```

This is why when we calculated the total cwnd growth per RTT, we multiplied the per-ACK increment by W:

```
Slow Start:
  Per-ACK increment    = MSS = 1,000 bytes
  Number of ACKs/RTT   = W = 4
  Total growth per RTT = 4 × 1,000 = 4,000 = cwnd  ← doubles

Congestion Avoidance:
  Per-ACK increment    = MSS²/cwnd = 1,000²/4,000 = 250 bytes
  Number of ACKs/RTT   = W = 4
  Total growth per RTT = 4 × 250 = 1,000 = MSS  ← always exactly 1 MSS
```

---

#### Summary

```
┌─────────────────────────────────────────────────────────┐
│  MSS = Maximum Segment Size = bytes per packet          │
│        the atomic unit of TCP data transmission         │
│                                                         │
│  W   = cwnd / MSS                                       │
│        window size expressed in packets, not bytes      │
│        just a notational convenience                    │
│                                                         │
│  Why W ACKs per RTT?                                    │
│    W packets in flight                                  │
│    → W packets delivered to receiver in one RTT         │
│    → W ACKs sent back by receiver                       │
│    → W ACKs received by sender within one RTT           │
│    one-to-one: every packet out = one ACK back          │
│                                                         │
│  This is why per-ACK increment × W = per-RTT growth     │
│  and why the two formulas (per-ACK vs per-RTT)          │
│  say exactly the same thing in different units          │
└─────────────────────────────────────────────────────────┘
```
---

### Q2： why it said Slow Start Exponentially grow congestion window to sense network capacity?


#### The slow start rule

```
Per-ACK increment = MSS    (fixed constant, does not depend on cwnd)
```

##### Sum over one RTT

If `cwnd = W × MSS`, then W ACKs arrive per RTT:

```
Total increase per RTT = W ACKs × MSS per ACK
                       = W × MSS
                       = cwnd
```

The window increases by **its own current size** every RTT. That is the definition of exponential growth.

```
RTT 1: cwnd = 1 MSS  → increases by 1 MSS  → cwnd = 2 MSS
RTT 2: cwnd = 2 MSS  → increases by 2 MSS  → cwnd = 4 MSS
RTT 3: cwnd = 4 MSS  → increases by 4 MSS  → cwnd = 8 MSS
RTT 4: cwnd = 8 MSS  → increases by 8 MSS  → cwnd = 16 MSS

cwnd(t) = MSS × 2^t
```

---

#### The critical difference between the two states

Now put them side by side and the contrast becomes exact:

```
Slow Start:
  Per-ACK increment = MSS              ← CONSTANT, does not change with cwnd
  ACKs per RTT     = W                 ← grows as cwnd grows
  Total per RTT    = W × MSS = cwnd    ← proportional to cwnd itself
  → growth rate ∝ current size → EXPONENTIAL

Congestion Avoidance:
  Per-ACK increment = MSS²/cwnd        ← SHRINKS as cwnd grows
  ACKs per RTT     = W                 ← grows as cwnd grows
  Total per RTT    = W × MSS²/cwnd = MSS ← constant regardless of cwnd
  → growth rate is constant → LINEAR
```

The entire difference comes down to one design choice:

```
Does the per-ACK increment shrink as cwnd grows?

  No  → more packets, same increment each → total grows with cwnd → exponential
  Yes → more packets, smaller increment   → total stays constant  → linear
```

---

#### Why exponential growth "senses" network capacity

The word "sense" in the slide is important. Think about what TCP is trying to do at startup:

```
TCP knows nothing about the network.
  Capacity could be 1 MSS. Could be 100,000 MSS.
  How do you find the right cwnd quickly?
```

##### The problem with linear probing from scratch

Suppose TCP used linear growth (+1 MSS per RTT) from the very beginning:

```
Actual capacity = 1,000 MSS

Linear growth:
  RTT 1:    cwnd = 1
  RTT 2:    cwnd = 2
  RTT 3:    cwnd = 3
  ...
  RTT 1000: cwnd = 1,000  ← finally reaches capacity

At RTT = 50ms: takes 1,000 × 50ms = 50 seconds to reach capacity.
The connection is badly underutilized for almost a minute.
```

##### Exponential search is much faster

```
Exponential growth:
  RTT 1:  cwnd = 1
  RTT 2:  cwnd = 2
  RTT 4:  cwnd = 4
  RTT 8:  cwnd = 8
  ...
  RTT 10: cwnd = 1,024  ← reaches capacity in log₂(1000) ≈ 10 RTTs

At RTT = 50ms: takes 10 × 50ms = 500ms.
100× faster than linear.
```

Exponential growth is essentially a **binary search through bandwidth space.** It starts small (safe) and doubles aggressively until it hits the boundary (a packet drop). The number of RTTs needed is logarithmic in the capacity:

```
RTTs to reach capacity C = log₂(C / MSS)

C = 10 MSS:        log₂(10)      ≈ 3 RTTs
C = 1,000 MSS:     log₂(1000)    ≈ 10 RTTs
C = 1,000,000 MSS: log₂(1000000) ≈ 20 RTTs
```

Even for enormous capacity, slow start finds it in ~20 RTTs. Linear growth would take 1,000,000 RTTs for that last case.

##### "Sense" means: overshoot deliberately, then back off

Slow start does not stop gently at capacity. It overshoots:

```
cwnd
 │                    × ← drop here (overshot capacity)
 │                 ●
 │              ●
 │           ●
 │        ●
 │     ●
 │  ●
 │●
 └──────────────────────────────────► RTT

Slow start grows until a packet is dropped.
The drop IS the signal that capacity was found.
ssthresh = cwnd/2 ← remember where the boundary is
cwnd = 1          ← restart, but now we know ssthresh
```

The drop is not a failure — it is the measurement. Slow start is deliberately probing until it breaks something, then using that information to set `ssthresh`. From then on, congestion avoidance grows linearly and carefully near that boundary.

---

#### The complete intuition in one picture

```
cwnd
 │                                      
 │   [Slow Start]          [CA]   drop  [Slow Start]  [CA]
 │       ●                               ●
 │     ●   ●                           ●   ●
 │   ●       ●                       ●       ●
 │ ●           ●─────────────●     ●           ●──────────
 │              ↑ ssthresh   ↑   ●              ↑ssthresh
 │                          drop               (= cwnd/2)
 └──────────────────────────────────────────────────────► RTT

Slow Start:          exponential — find capacity fast via logarithmic search
Congestion Avoidance: linear     — probe carefully near known boundary
ssthresh:            the memory  — where congestion was last seen
```

```
┌────────────────────────────────────────────────────────────┐
│  Why exponential in slow start?                            │
│    Per-ACK increment is FIXED (MSS)                        │
│    More ACKs × same increment = total grows with cwnd      │
│    → cwnd doubles every RTT                                │
│    → finds capacity in log₂(C) RTTs, not C RTTs           │
│                                                            │
│  Why linear in congestion avoidance?                       │
│    Per-ACK increment SHRINKS (MSS²/cwnd)                   │
│    More ACKs × smaller increment = constant total          │
│    → cwnd grows by exactly 1 MSS per RTT always            │
│    → careful additive probing near the capacity boundary   │
└────────────────────────────────────────────────────────────┘
```

---





### Q3： why it  Congestion Avoidance  produces **linear growth**?
Good question. Let me derive it step by step so the math is completely transparent.
#### The rule for Congestion Avoidance

In congestion avoidance, for every ACK received:

```
cwnd += MSS² / cwnd
```

This looks like it would produce different growth rates depending on `cwnd`. Let me show you why it always produces exactly **+1 MSS per RTT** regardless of `cwnd`.

---


#### Now sum up the total increase over one RTT


```
cwnd =  W * MSS

so the number of ACKs per RTT equals the window size in packets:

ACKs per RTT = cwnd / MSS = W
```


Each of those W ACKs triggers:

```
cwnd += MSS² / cwnd
```

Total increase over one full RTT = sum over all W ACKs:

```
Total increase = W × (MSS² / cwnd)
               = W × (MSS² / (W × MSS))
               = W × (MSS / W)
               = MSS
```

The W cancels exactly. The total increase is always **1 MSS per RTT**, no matter what W is.



---

#### Concrete example to verify

Let MSS = 1,000 bytes. Run through three different window sizes:

```
Case 1: cwnd = 4,000 bytes (W = 4 packets)
  Per-ACK increase = MSS²/cwnd = 1,000²/4,000 = 250 bytes
  ACKs per RTT    = 4
  Total per RTT   = 4 × 250 = 1,000 bytes = 1 MSS  ✓

Case 2: cwnd = 10,000 bytes (W = 10 packets)
  Per-ACK increase = MSS²/cwnd = 1,000²/10,000 = 100 bytes
  ACKs per RTT    = 10
  Total per RTT   = 10 × 100 = 1,000 bytes = 1 MSS  ✓

Case 3: cwnd = 100,000 bytes (W = 100 packets)
  Per-ACK increase = MSS²/cwnd = 1,000²/100,000 = 10 bytes
  ACKs per RTT    = 100
  Total per RTT   = 100 × 10 = 1,000 bytes = 1 MSS  ✓
```

Larger `cwnd` → smaller per-ACK increment, but more ACKs arrive → always nets out to exactly 1 MSS.

---

#### Why the formula is designed this way

The design goal was: **additive increase of exactly 1 MSS per RTT.**

But TCP does not have a per-RTT timer firing to increment cwnd. It only has per-ACK events. So the question becomes: given that W ACKs arrive per RTT, what per-ACK increment produces a total of +1 MSS per RTT?

```
W ACKs × (increment per ACK) = 1 MSS

increment per ACK = 1 MSS / W
                  = 1 MSS / (cwnd / MSS)
                  = MSS² / cwnd
```

That is exactly where the formula comes from. It is deliberately constructed so that per-ACK increments sum to exactly 1 MSS over one RTT — producing the constant linear growth the AIMD algorithm requires.

---

#### The contrast with slow start

Now it is clear why slow start grows exponentially and congestion avoidance grows linearly:

```
Slow Start:
  Per-ACK increment = MSS           (fixed, does not shrink with cwnd)
  ACKs per RTT     = W
  Total per RTT    = W × MSS = cwnd → cwnd doubles every RTT (exponential)

Congestion Avoidance:
  Per-ACK increment = MSS²/cwnd     (shrinks as cwnd grows)
  ACKs per RTT     = W
  Total per RTT    = W × MSS²/cwnd = MSS → always +1 MSS (linear)
```

The difference is entirely in what happens to the per-ACK increment as `cwnd` grows:

```
Slow start:           increment stays CONSTANT as cwnd grows
                      → more ACKs × same increment = accelerating total
                      → exponential

Congestion avoidance: increment SHRINKS proportionally as cwnd grows
                      → more ACKs × smaller increment = constant total
                      → linear
```

That is the complete explanation. The `MSS²/cwnd` formula is not arbitrary — it is the precise mathematical correction that keeps the growth rate constant at 1 MSS per RTT regardless of the current window size.



## 10. TCP Tahoe FSM

###  State Transitions

#### Two goals
- Use slow start to quickly find network capacity
- When close to capacity, use congestion avoidance to very carefully probe

#### Three signals TCP observes

| Signal | Meaning |
|---|---|
| Increasing ACK | Transfer is going well; advance window |
| Duplicate ACKs | A segment was lost, but others are arriving |
| Timeout | Something is very wrong; network may be broken |



#### The `ssthresh` variable

`ssthresh` (slow start threshold) is the boundary between the two states:

- If `cwnd < ssthresh`: use Slow Start (exponential)
- If `cwnd ≥ ssthresh`: use Congestion Avoidance (linear)

When a loss is detected: `ssthresh = cwnd / 2`

This means TCP remembers where trouble started and becomes conservative near that point.


### TCP Tahoe State Machine

```
                                     cwnd > ssthresh                                  
┌──────────────────────────────┐                    ┌──────────────────────────────┐                          
│         Slow Start           │                    │      Congestion Avoidance    │
│   ack: cwnd += MSS           │                    │   ack: cwnd += MSS²/cwnd     │
└──────────────────────────────┘                    └──────────────────────────────┘
                              On timeout OR triple dup ACK:
                                ssthresh = cwnd / 2
                                cwnd = 1 → return to Slow Start
```

### TCP Tahoe behavior over time

```
cwnd
│                  ssthresh
│    /\              ▼
│   /  \           /
│  /    \         /
│ /      │       /
│/   dup │      /
│    ack │     /
│        \    /
│     timeout\/
│
└──────────────────────────────► time

Phase 1: Slow start (exponential)
Phase 2: Drop on dup ack → cwnd=1, slow start again
Phase 3: Reaches ssthresh → congestion avoidance (linear)
Phase 4: Timeout → cwnd=1 again
```

---



### Q1： Explain how TCP Congestion Window work?
Let me build this from the ground up — why it exists, what it does, and how it works mechanically.

---

#### Why the congestion window exists

Before TCP Tahoe, TCP only had one constraint on how much data to send — the **flow control window** (advertised by the receiver in the TCP header).

```
Receiver says: "I have 64 KB of buffer space"
Sender thinks: "Great, I'll send 64 KB immediately"

Problem: the receiver can handle 64 KB,
         but the NETWORK between them cannot.
```

The flow control window answers: *"how much can the receiver handle?"*
The congestion window answers: *"how much can the network handle?"*

These are two completely independent questions:

```
                    flow control window
                    (receiver's buffer)
Sender ──────────────────────────────────────────► Receiver
         Router1      Router2      Router3
           ↑             ↑            ↑
           these routers have their own queues
           the receiver knows nothing about them
           the congestion window estimates THEIR capacity
```

So TCP maintains both and takes the minimum:

```
Sender window = min(flow control window, congestion window)
                     ════════════════    ════════════════
                     don't overwhelm     don't overwhelm
                     the receiver        the network
```

---

#### What the congestion window physically means

`cwnd` is the maximum number of **unacknowledged bytes** the sender is allowed to have in the network at any moment.

```
Sender can have at most cwnd bytes "in flight":

  sent but not yet ACKed:
  ┌────┬────┬────┬────┬────┬────┐
  │ p1 │ p2 │ p3 │ p4 │ p5 │p6 │  ← all in flight
  └────┴────┴────┴────┴────┴────┘
  ←────────── cwnd ──────────────►

  Cannot send p7 until an ACK arrives and frees space.
```

This is exactly the bandwidth-delay product concept — `cwnd` is TCP's estimate of how many bytes the pipe can hold.

---

#### The two states

TCP Tahoe separates congestion control into two states with very different behaviors. The key distinction is **how fast `cwnd` grows**.

##### State 1: Slow Start

**When:** connection startup, or after a timeout.

**Goal:** find the network capacity quickly without knowing it in advance.

**Rule:** for every ACK received, increase `cwnd` by 1 MSS.

```
cwnd += MSS    (per ACK)
```

This produces **exponential growth**:

```
RTT 1: cwnd=1, send 1 packet → receive 1 ACK  → cwnd=2
RTT 2: cwnd=2, send 2 packets → receive 2 ACKs → cwnd=4
RTT 3: cwnd=4, send 4 packets → receive 4 ACKs → cwnd=8
RTT 4: cwnd=8, send 8 packets → receive 8 ACKs → cwnd=16
...

cwnd doubles every RTT.
```

Visually:

```
cwnd
 │                      ●
 │                 ●
 │            ●
 │       ●
 │   ●
 │ ●
 │●
 └──────────────────────────► RTT
   1  2  3  4  5  6  7
```

**Why exponential and not linear?** Because the network capacity might be 1 MSS or it might be 10,000 MSS. Linear growth from 1 MSS would take far too long to find the right operating point. Exponential growth gets there quickly.

**Why is it called "slow"?** Not because growth is slow — exponential is fast. It is slow compared to the *old* pre-Tahoe approach of blasting the entire flow control window immediately. Starting at 1 MSS and doubling is much gentler than sending 64 KB in one burst.

##### State 2: Congestion Avoidance

**When:** `cwnd` exceeds `ssthresh` (more on this below).

**Goal:** carefully probe for more bandwidth without causing congestion.

**Rule:** for every ACK received, increase `cwnd` by `MSS²/cwnd`.

```
cwnd += MSS²/cwnd    (per ACK)
```

This produces **linear growth** — exactly 1 MSS per RTT:

```
If cwnd = W (in MSS units), then W ACKs arrive per RTT.
Each adds MSS²/cwnd = MSS²/(W × MSS) = MSS/W

Total increase per RTT = W × (MSS/W) = MSS

Always +1 MSS per RTT, regardless of cwnd size.
```

Visually:

```
cwnd
 │                              ●
 │                         ●
 │                    ●
 │               ●
 │          ●
 │     ●
 └──────────────────────────► RTT
```

This is the **additive increase** part of AIMD. Linear, steady, cautious.

---

#### The `ssthresh` variable: memory of past congestion

`ssthresh` (slow start threshold) is the boundary between the two states.

```
if cwnd < ssthresh:   → Slow Start   (exponential growth)
if cwnd ≥ ssthresh:   → Congestion Avoidance (linear growth)
```

**How ssthresh gets set:**

```
Initial value:    very large (effectively ∞)
                  → TCP starts in slow start and grows until it hits trouble

On any loss event:
  ssthresh = cwnd / 2
  → TCP remembers: "last time I was at cwnd, there was congestion.
                    Be careful near cwnd/2 next time."
```

This is TCP's **memory of where the network boundary is.** It does not start slow start from scratch every time — it uses ssthresh to switch to careful linear growth as it approaches the danger zone.

---

#### The complete lifecycle

Putting it all together:

```
cwnd
 │
 │    Slow Start         CA        drop    Slow Start    CA
 │        ●                          ↓         ●
 │      ●   ●                        │       ●   ●
 │    ●       ●                      │     ●       ●
 │  ●           ●──────────●         │   ●           ●─────────
 │●              ssthresh  │         │ ●              ssthresh
 │                         drop      │
 └─────────────────────────────────────────────────────────► time

Phase 1: Slow start from cwnd=1, exponential growth
Phase 2: Hit ssthresh → switch to congestion avoidance (linear)
Phase 3: Packet loss → ssthresh=cwnd/2, cwnd=1, back to slow start
Phase 4: Slow start again, but now ssthresh is set lower
Phase 5: Hit new (lower) ssthresh → congestion avoidance again
```

##### What triggers a loss event?

Two signals, two different reactions in TCP Tahoe:

```
Signal 1: Timeout
  → Something is seriously wrong
  → ssthresh = cwnd / 2
  → cwnd = 1
  → Enter slow start

Signal 2: Triple duplicate ACK (fast retransmit)
  → One packet lost, but others are arriving
  → ssthresh = cwnd / 2
  → cwnd = 1
  → Enter slow start
  (TCP Reno improves this — cwnd = ssthresh instead)
```

---

#### A concrete numerical walkthrough

MSS = 1 KB, initial ssthresh = 16 KB:

```
RTT  State   cwnd    Event
───  ──────  ──────  ──────────────────────────────
 1   SS       1 KB   start
 2   SS       2 KB
 3   SS       4 KB
 4   SS       8 KB
 5   SS      16 KB   cwnd reached ssthresh → switch to CA
 6   CA      17 KB
 7   CA      18 KB
 8   CA      19 KB
 9   CA      20 KB   packet dropped!
         ssthresh = 20/2 = 10 KB
         cwnd = 1 KB → back to slow start
10   SS       1 KB
11   SS       2 KB
12   SS       4 KB
13   SS       8 KB
14   SS      10 KB   cwnd reached new ssthresh → switch to CA
15   CA      11 KB
16   CA      12 KB
...
```

Notice how the second time through, ssthresh = 10 KB instead of 16 KB. TCP learned from the first congestion event and becomes conservative earlier.

---

#### Why two states and not just one?

A natural question: why not just always use linear growth (congestion avoidance)?

```
Problem: linear growth from 1 MSS is far too slow to reach capacity.

Example: 100 Mb/s link, RTT = 50 ms, MSS = 1500 bytes
  BDP = 100 Mb/s × 50 ms = 5,000,000 bits = ~416 MSS

  Linear growth: takes 416 RTTs = 416 × 50 ms = ~20 seconds
                 to reach full capacity from scratch.

  Exponential growth (slow start): takes log₂(416) ≈ 9 RTTs = 450 ms
                                   to reach full capacity.
```

On the other hand, why not always use exponential growth?

```
Problem: exponential growth near the capacity boundary is too aggressive.

  cwnd = 200 MSS, capacity ≈ 210 MSS
  Next RTT: cwnd doubles to 400 MSS
  Far overshoots the boundary → massive queue overflow → many packets dropped
```

The two-state design solves both problems:

```
Far below capacity:   Slow Start (exponential) → get there fast
Near capacity:        Congestion Avoidance (linear) → probe carefully
```

`ssthresh` is the dividing line between "far below" and "near capacity" — updated dynamically from each congestion event.

---

#### Summary

```
┌────────────────────────────────────────────────────────────────┐
│                   TCP Congestion Window                        │
│                                                                │
│  cwnd = estimate of how many bytes the network can hold        │
│  Sender window = min(cwnd, flow control window)                │
│                                                                │
│  Slow Start (cwnd < ssthresh):                                 │
│    cwnd += MSS per ACK → doubles every RTT                     │
│    Goal: reach capacity quickly from a cold start              │
│                                                                │
│  Congestion Avoidance (cwnd ≥ ssthresh):                       │
│    cwnd += MSS²/cwnd per ACK → +1 MSS per RTT                  │
│    Goal: additive increase, probe carefully near capacity      │
│                                                                │
│  On loss:                                                      │
│    ssthresh = cwnd / 2   (remember the danger zone)            │
│    cwnd = 1              (back to slow start)                  │
│                                                                │
│  ssthresh = TCP's memory of where congestion last occurred     │
│           = the boundary between fast and careful growth       │
└────────────────────────────────────────────────────────────────┘
```

---


## 11. RTT Estimation


### Why it matters

The retransmission timeout (RTO) is critical:

| RTO too short | RTO too long |
|---|---|
| False retransmissions | Long idle periods before retransmit |
| Unnecessary slow starts | Wasted capacity |
| Reduced throughput | Poor performance on lossy links |

### Pre-Tahoe: Exponential Weighted Moving Average (EWMA)

r is RTT estimate, initialize to something reasonable
m, RTT measurement from most recently acked data packet

Exponentially weighted moving average:
```
r  = α × r + (1 - α) × m      (EWMA of RTT samples)
RTO = 2 × r
```

**Problem:** This assumes variance is a constant fraction of the mean. It fails when:
- RTT is high but stable (RTO is unnecessarily large, wasting time)
- RTT is low but highly variable (RTO is too small, triggering false timeouts)

### TCP Tahoe: EWMA + Variance Tracking

r is RTT estimate, initialize to something reasonable
g is the EWMA gain (e.g., 0.25)
m is the RTT measurement from most recently acked data packet

```
e   = m - r                   (error between measurement and estimate)
r   = r + g × e               (update mean, g = 0.125 typically)
v   = v + g × (|e| - v)       (update variance estimate)
RTO = r + 4 × v               (timeout = mean + 4 standard deviations)
```

**Key insight:** By tracking variance separately, the timeout automatically adapts:

- Low-variance connection → RTO ≈ RTT + small margin
- High-variance connection → RTO = RTT + large margin

**Additional rule:** If a retransmission itself times out, exponentially back off the timer — TCP assumes the network is severely congested.


### Q1: Why we need RTT estimation when congestion control?

Great question. Let me build the answer from first principles.

---

#### The core problem: TCP needs to know when a packet is lost

TCP's congestion control reacts to packet loss. But how does a sender know a packet was lost? It never receives a direct "your packet was dropped" message from the network. The only signal it has is **silence** — the ACK never comes back.

So TCP sets a timer when it sends a packet. If the timer fires before the ACK arrives, TCP concludes the packet was lost and retransmits.

That timer is the **Retransmission Timeout (RTO)**. And to set it correctly, TCP needs to know the RTT.

```
Sender transmits packet
│
│ ← starts RTO timer
│
│  If ACK arrives before timer fires:  packet delivered ✓
│  If timer fires before ACK arrives:  assume packet lost → retransmit
│
└── RTO must be set correctly or everything breaks
```

---

#### What happens if RTO is wrong?

#### RTO too short

The timer fires before the ACK has had time to return, even though the packet was delivered successfully.

```
Sender                          Receiver
  │──── pkt ──────────────────►│
  │  ← RTO fires too early      │
  │──── retransmit ────────────►│  (duplicate! wasted bandwidth)
  │                              │──── ACK ──────────────────────►│
  │◄──── ACK ────────────────── │  (original ACK arrives late)
```

Consequences:
- **Unnecessary retransmissions** waste bandwidth
- **False congestion signal** — TCP thinks a packet was lost, so it halves cwnd and enters slow start
- The network is not actually congested, but TCP throttles itself anyway
- Throughput collapses for no reason

##### RTO too long

The timer is set so conservatively that TCP waits a long time before concluding a packet was lost.

```
Sender                          Receiver
  │──── pkt ──────────────────►│ (dropped in router)
  │                              │
  │  ← waiting... waiting...     │
  │  ← waiting... waiting...     │  (link sits idle the whole time)
  │  ← RTO finally fires         │
  │──── retransmit ────────────►│
```

Consequences:
- **Long idle periods** — the link carries nothing while TCP waits
- Throughput drops because the sender is stalled
- Particularly bad on high-latency links where even a correct RTO is already large

---

#### Why is RTO hard to set?

You might think: just measure the RTT once and use that. The problem is RTT is not constant. It varies continuously because:

```
RTT = propagation delay + queueing delay + transmission delay

Propagation:   fixed by physics — stable
Transmission:  fixed by packet size and link rate — stable
Queueing:      depends on how full router buffers are — highly variable
```

Queueing delay changes with every burst of traffic from every other flow sharing the path. On a busy link it might add 50ms. On a quiet moment it might add 1ms.

```
RTT samples over time (ms):
  20, 22, 19, 21, 45, 23, 20, 80, 21, 19, ...
                    ↑               ↑
               brief queue      large queue
               spike            spike

If RTO = 2 × average = 2 × 27ms = 54ms:
  → the 80ms sample triggers a false timeout
  → unnecessary retransmit + slow start
```

So TCP needs to **track both the typical RTT and how much it varies**, and set RTO accordingly.

---

#### Pre-Tahoe estimation: EWMA only

Before Van Jacobson's fix, TCP used a simple exponential weighted moving average:

```
r   = α × r + (1 - α) × m      (α = 0.875 typically)
RTO = 2 × r
```

where `m` is the latest RTT measurement and `r` is the smoothed estimate.

**Problem:** `2 × r` assumes the variance is always a fixed fraction of the mean. 

This fails in two opposite ways:

```
Case 1: High mean, low variance (e.g. satellite link, stable 600ms RTT)
  r = 600ms, RTO = 1200ms
  But actual RTTs are always 598–602ms.
  Wasting ~600ms waiting before retransmitting. Link sits idle.

Case 2: Low mean, high variance (e.g. congested urban link)
  r = 20ms, RTO = 40ms
  But some RTTs spike to 80ms legitimately.
  False timeouts → unnecessary slow starts → throughput collapse.
```

The multiplier `2×` is just a guess that works sometimes and fails badly in others.

---

#### TCP Tahoe's fix: track variance explicitly

Van Jacobson's insight: **measure how much RTT samples deviate from the estimate, and adapt the timeout to that deviation.**

```
Each time a new ACK arrives with RTT sample m:

  e = m - r                     (error between sample and estimate)
  r = r + g × e                 (update mean,     g ≈ 0.125)
  v = v + g × (|e| - v)         (update variance, same gain)
  RTO = r + 4 × v               (mean + 4 standard deviations)
```

The `4v` term is the key addition. It makes RTO **self-adapting**:

```
Stable RTT (low variance):
  v → 0
  RTO → r + 0 = r       ← tight, just above the mean
  No wasted waiting time.

Variable RTT (high variance):
  v → large
  RTO → r + 4v          ← wide margin, covers the spikes
  No false timeouts.
```

Visualized:

```
RTT samples:   ──●──●──●──●──●──●──●──●──   (stable, low variance)
RTO:           ════════════════════════════   (just above samples, tight)

RTT samples:   ──●──●──────────●──●──●──●─  (variable, spikes)
                          ↑ spike
RTO:           ════════════════════════════   (wider, covers spikes)
```

---

#### The connection to congestion control

RTT estimation is not just about retransmission. It feeds directly into congestion control in several ways:

##### 1. False timeouts trigger unnecessary slow starts

Without good RTT estimation, a legitimate RTT spike (caused by a temporarily full queue) triggers a timeout. TCP then:

```
false timeout → cwnd = 1 → slow start → throughput collapses

But the queue is fine. No packet was actually lost.
The congestion control reaction was completely unnecessary.
```

Good RTT estimation prevents this by setting RTO wide enough to tolerate legitimate spikes.

##### 2. RTT measurement drives the additive increase rate

The AIMD rule is: **+1 MSS per RTT**. TCP's sender uses its RTT estimate to know how quickly it is increasing:

```
If RTT estimate is wrong:
  Underestimate RTT → thinks it's increasing faster than it is → overaggressive
  Overestimate RTT → increases more slowly than intended → underutilizes link
```

##### 3. Distinguishing congestion from delay

A packet with a very long RTT sample might mean:
- The packet was queued for a long time (congestion building)
- The ACK was delayed on the return path
- Random variation

Good variance tracking lets TCP distinguish "this is normal variation" from "something is wrong."

##### 4. The exponential backoff rule

If a retransmission itself times out (the retransmitted packet is also lost), TCP doubles the RTO:

```
1st timeout:  RTO = r + 4v
2nd timeout:  RTO = 2 × (r + 4v)
3rd timeout:  RTO = 4 × (r + 4v)
...
```

This exponential backoff prevents TCP from flooding an already severely congested network with rapid retransmissions. Without RTT estimation giving a sensible base RTO, the backoff has no meaningful starting point.

---

#### Summary

```
┌────────────────────────────────────────────────────────────┐
│         Why RTT estimation is essential                    │
│                                                            │
│  TCP detects loss via TIMEOUT — no explicit signal         │
│                                                            │
│  RTO too short → false timeouts                            │
│    → unnecessary retransmits                               │
│    → unnecessary slow starts                               │
│    → throughput collapse despite no real congestion        │
│                                                            │
│  RTO too long → late retransmits                           │
│    → long idle periods                                     │
│    → poor throughput on lossy links                        │
│                                                            │
│  RTT is not constant — queueing delay varies continuously  │
│                                                            │
│  Pre-Tahoe: EWMA only → fails on high or variable RTT      │
│  TCP Tahoe: EWMA + variance → RTO = mean + 4σ              │
│    → tight when RTT is stable                              │
│    → wide when RTT is variable                             │
│    → self-adapting, works across diverse network paths     │
└────────────────────────────────────────────────────────────┘
```

In one sentence: **without accurate RTT estimation, TCP cannot tell the difference between a lost packet and a slow one — and gets the congestion control reaction wrong in either direction.**


### Q2: why you said 2 × r assumes the variance is always a fixed fraction of the mean?

Great question — this is a subtle but important point. Let me unpack it carefully.

---

#### What `RTO = 2 × r` is implicitly assuming

When you write `RTO = 2 × r`, you are saying:

```
RTO = r + r

In other words:
RTO = mean + safety margin
            ↑
        this safety margin = r = 1× the mean
```

The safety margin exists to cover the variance — the spread of RTT samples above the mean. If RTT samples sometimes run 30% above the mean, your margin needs to cover that. If they sometimes run 200% above the mean, your margin needs to be much larger.

By hardcoding the margin as exactly `1 × r`, you are implicitly claiming:

```
safety margin needed ∝ r (the mean)
↕
variance of RTT ∝ mean of RTT
```

That is the hidden assumption. The formula treats variance as always being a fixed proportion of the mean — specifically, 100% of the mean.

---

#### Why that assumption is wrong

There is no reason the variance of RTT should be proportional to the mean. They are independent properties of the network path. Consider:

```
Path A: Satellite link
  Mean RTT     = 600 ms   (long propagation delay to satellite)
  Variance     = 2 ms     (very stable — satellite link is rarely congested)

  2 × r = 1200 ms
  Actual spread of samples: 598–602 ms

  You are waiting 1200 ms before retransmitting,
  when you could safely retransmit at ~605 ms.
  Wasting ~595 ms of idle time per loss event.

Path B: Congested urban link
  Mean RTT     = 20 ms    (short propagation)
  Variance     = 30 ms    (heavily variable queueing delay)

  2 × r = 40 ms
  Actual spread of samples: 5–80 ms

  You set RTO = 40 ms, but legitimate packets sometimes
  take 80 ms. You retransmit them as "lost" when they
  are still in flight → false timeout.
```

In Path A, the mean is large but variance is tiny. In Path B, the mean is small but variance is huge. The `2 ×` multiplier cannot handle both cases because it conflates two independent quantities.

---

#### The deeper statistical point

In some distributions, variance really is proportional to the mean. For example, in a Poisson distribution, `variance = mean`. If someone assumed RTT followed a Poisson distribution, `2 × r` would make sense as a rough `mean + 1σ` approximation.

But RTT does not follow a Poisson distribution. Queueing delay has its own dynamics — it can be near-zero when queues are empty and spike suddenly when a burst arrives. The mean and variance of RTT are essentially unrelated:

```
RTO = 2 × r
    = r + 1 × r
    = mean + (1 × mean) as proxy for variance

This works only if:   actual variance ≈ mean
This fails when:      variance << mean  (stable high-latency path)
                      variance >> mean  (variable low-latency path)
```

---

#### What Tahoe does instead

Tahoe measures variance **directly** and independently of the mean:

```
e   = m - r              (how far this sample is from the mean)
v   = v + g(|e| - v)     (EWMA of the absolute deviation)
RTO = r + 4 × v          (mean + 4 × measured spread)
```

Now the safety margin `4v` tracks actual observed variance — it is large when RTT is variable, small when RTT is stable. The mean and variance are estimated separately, so they can move independently. The `2 ×` multiplier assumed they were locked together; Tahoe breaks that assumption.

```
Pre-Tahoe:   RTO = mean + fixed_fraction × mean
                                ↑
                     variance assumed = mean (wrong)

TCP Tahoe:   RTO = mean + 4 × measured_variance
                                ↑
                     variance measured directly (correct)
```

That is precisely why Tahoe's RTO adapts correctly to both the satellite case (small v → tight RTO) and the congested urban case (large v → wide RTO), while pre-Tahoe fails at one or both.

---



### Q3 ： how to interpret TCP Tahoe: EWMA + Variance Tracking and the paramter g?

Great question. Let me build this from the ground up — what EWMA is, why it works, and what `g` controls.

---

#### First: why not just use the latest sample?

The naive approach: measure one RTT sample `m`, set `RTO = 2 × m`.

```
RTT samples:  20, 21, 19, 80, 20, 22, 19 ms
                          ↑
                     one spike

RTO would jump to 160 ms on the spike,
then drop back to ~40 ms next sample.

Problem: one unusual sample completely dominates.
The estimate is too noisy to be useful.
```

You need a way to **smooth out noise** while still **tracking real changes**. That is exactly what EWMA does.

---

#### What EWMA is: a weighted memory

EWMA = Exponentially Weighted Moving Average. The formula:

```
r  ←  r + g × (m - r)
     ═══  ════════════
      ↑        ↑
  keep most   nudge toward
  of old      new sample
  estimate    by a little
```

Rewritten equivalently:

```
r  ←  (1 - g) × r  +  g × m
       ═══════════     ═════
        old estimate   new sample
        weighted by    weighted by
        (1 - g)        g
```

This is why it is called "exponentially weighted" — the influence of past samples decays exponentially. A sample from 1 RTT ago has weight `g`. From 2 RTTs ago: `g(1-g)`. From 3 RTTs ago: `g(1-g)²`. And so on:

```
Influence of past samples (g = 0.125):

  1 RTT ago:   g           = 0.125       = 12.5%
  2 RTTs ago:  g(1-g)      = 0.109       = 10.9%
  3 RTTs ago:  g(1-g)²     = 0.095       =  9.5%
  4 RTTs ago:  g(1-g)³     = 0.083       =  8.3%
  ...
  10 RTTs ago: g(1-g)⁹     = 0.040       =  4.0%
  20 RTTs ago: g(1-g)¹⁹    = 0.011       =  1.1%

Old samples fade away. Recent samples dominate.
```

---

#### What `g` controls: the memory/responsiveness tradeoff

`g` is the **gain** — how strongly the new sample pulls the estimate.

```
Large g (e.g. 0.5):          Small g (e.g. 0.01):
  Fast to respond              Slow to respond
  Trusts new samples heavily   Trusts new samples lightly
  Noisy estimate               Very smooth estimate
  Forgets history quickly      Long memory of past

         g = 0           g = 0.125         g = 1
         │                   │               │
         ▼                   ▼               ▼
    never updates        TCP Tahoe       = latest sample
    r stays fixed        sweet spot      no smoothing
```

Concretely, with `g = 0.125`:

```
New sample m arrives.
r moves only 12.5% of the way toward m.
The other 87.5% stays where it was.

Example:
  r = 20 ms, sudden spike m = 80 ms
  e = 80 - 20 = 60 ms
  r ← 20 + 0.125 × 60 = 20 + 7.5 = 27.5 ms

  One spike moves r by only 7.5 ms, not 60 ms.
  The estimate is stable despite the spike.
```

The "effective memory" — roughly how many past samples influence the estimate — is approximately:

```
Memory ≈ 1/g samples

g = 0.125  →  memory ≈ 8 samples
g = 0.25   →  memory ≈ 4 samples
g = 0.01   →  memory ≈ 100 samples
```

With `g = 0.125`, the estimate reflects roughly the last 8 RTT measurements, with recent ones weighted more heavily.

---

#### Now: the variance estimate `v`

The same EWMA principle applied to the **error magnitude** `|e|`:

```
e = m - r                   (signed error: how far sample is from mean)
v ← v + g × (|e| - v)      (EWMA of absolute error)
```

This is identical in structure to the mean update:

```
Mean update:      r ← r + g × (m   - r)    tracking: raw samples m
Variance update:  v ← v + g × (|e| - v)    tracking: absolute deviations |e|
```

`v` is an exponentially weighted average of `|m - r|` — how far each sample strays from the running mean. It is a measure of **spread**, closely related to mean absolute deviation (MAD).

#### Why `|e|` and not `e²`?

Standard statistics uses variance = average of `e²`. Van Jacobson chose `|e|` (absolute deviation) instead because:

```
|e| is cheaper to compute    — no multiplication needed on 1980s hardware
|e| is less sensitive        — squaring amplifies outliers heavily
Both track spread effectively — either works for this purpose
```

---

##### Putting it together: the full update cycle

Each time an ACK arrives with new RTT measurement `m`:

```
Step 1: compute error
  e = m - r

  e > 0: sample is above the mean (RTT spiked)
  e < 0: sample is below the mean (RTT improved)
  e = 0: sample matches estimate exactly

Step 2: update mean
  r ← r + g × e
  (nudge mean toward the new sample by fraction g)

Step 3: update variance
  v ← v + g × (|e| - v)
  (nudge variance toward the new absolute deviation by same fraction g)

Step 4: compute RTO
  RTO = r + 4 × v
```

##### A worked numerical example

Starting state: `r = 20 ms`, `v = 2 ms`, `g = 0.125`

```
Sample 1: m = 22 ms (slightly above mean)
  e   = 22 - 20 = +2
  r   = 20 + 0.125 × 2        = 20.25 ms
  v   = 2  + 0.125 × (2 - 2)  = 2.00  ms
  RTO = 20.25 + 4 × 2.00      = 28.25 ms

Sample 2: m = 19 ms (slightly below mean)
  e   = 19 - 20.25 = -1.25
  r   = 20.25 + 0.125 × (-1.25) = 20.09 ms
  v   = 2.00 + 0.125 × (1.25 - 2.00) = 1.91 ms
  RTO = 20.09 + 4 × 1.91      = 27.73 ms
  (stable RTT → RTO stays tight)

Sample 3: m = 80 ms (sudden spike — queue burst)
  e   = 80 - 20.09 = +59.91
  r   = 20.09 + 0.125 × 59.91 = 27.58 ms
  v   = 1.91 + 0.125 × (59.91 - 1.91) = 9.16 ms
  RTO = 27.58 + 4 × 9.16      = 64.22 ms
  (spike detected → v jumps → RTO widens immediately)

Sample 4: m = 21 ms (back to normal)
  e   = 21 - 27.58 = -6.58
  r   = 27.58 + 0.125 × (-6.58) = 26.76 ms
  v   = 9.16 + 0.125 × (6.58 - 9.16) = 8.84 ms
  RTO = 26.76 + 4 × 8.84      = 62.12 ms
  (v decays slowly — RTO stays wide for a while after spike)
```

Notice: after the spike, `v` does not immediately drop back. It decays gradually over subsequent samples, keeping RTO wide for a few RTTs. This is intentional — one normal sample after a spike does not mean the spike won't recur.

---

#### Why `4 × v` specifically?

```
RTO = r + 4 × v
```

The factor 4 is a practical engineering choice, not a derived constant. The reasoning:

```
For a normal distribution:
  mean ± 2σ covers 95% of samples
  mean ± 3σ covers 99.7% of samples
  mean ± 4σ covers 99.994% of samples

With RTO = r + 4v, the vast majority of legitimate ACKs
arrive before the timer fires, even with high variance.
False timeout rate becomes very low.
```

Note: `v` here is mean absolute deviation, not standard deviation `σ`. For a normal distribution, `σ ≈ 1.25 × MAD`, so `4v ≈ 3.2σ` — roughly equivalent to 3σ coverage, which is very conservative. Van Jacobson chose 4 after empirical testing on real networks in the 1980s. It works well in practice across a wide variety of network conditions.

---

#### Summary: the full picture

```
┌──────────────────────────────────────────────────────────────┐
│                  TCP Tahoe RTT Estimation                    │
│                                                              │
│  g = 0.125  →  memory ≈ 8 samples                           │
│              →  smooth but responsive                        │
│                                                              │
│  r  tracks the mean RTT                                      │
│     moves slowly, stable against noise                       │
│     reacts to sustained RTT changes                          │
│                                                              │
│  v  tracks the spread of RTT samples                         │
│     rises quickly when samples become variable               │
│     decays slowly back when samples stabilize                │
│                                                              │
│  RTO = r + 4v                                                │
│     tight  when v is small  (stable path)                    │
│     wide   when v is large  (variable/congested path)        │
│     self-adapting — no hardcoded multiplier of the mean      │
│                                                              │
│  Both r and v use the SAME gain g — one knob to tune,        │
│  both tracking at the same time scale.                       │
└──────────────────────────────────────────────────────────────┘
```

The elegance of this design is that it needs only two state variables (`r` and `v`), one tuning parameter (`g`), and four arithmetic operations per ACK — yet it handles the full range of network conditions from stable satellite links to jittery urban connections.


## 12. Self-Clocking

### The principle

> TCP sends acknowledgements as quickly as possible. This leads to ACKs being naturally spaced in time according to the bottleneck link rate — which in turn causes the sender to transmit at exactly the bottleneck rate.

### How it works

```
Sender                  Bottleneck              Receiver
  │  ─── pkt 1 ─────────►│                         │
  │  ─── pkt 2 ─────────►│  (pkt 1 queued)         │
  │  ─── pkt 3 ─────────►│  (pkt 2 queued)         │
  │                        │──► pkt 1 ─────────────►│
  │◄─── ACK 1 ────────────────────────────────────  │
  │  send pkt 4           │──► pkt 2 ─────────────►│
  │◄─── ACK 2 ────────────────────────────────────  │
  │  send pkt 5           │──► pkt 3 ─────────────►│
  ...
```

Even if the sender blasts packets in a burst, the bottleneck queues them and releases them one by one at the link rate. ACKs flow back at that same spaced rate. The sender then sends one new packet per ACK — perfectly matching the bottleneck rate.

### Why this matters

1. **Prevents congestion:** TCP only puts a new packet into the network when a packet has left (via an ACK). The number of outstanding packets stays stable.
2. **Rate matching:** The sender automatically adapts to the bottleneck without any explicit rate signal.
3. **Emergent behavior:** No explicit rate negotiation needed — pacing emerges from ACK timing.

**Send ACKs immediately.** Delayed ACKs break self-clocking and hurt performance.

### Q1: How do Self-Clocking works?

Great question. Self-clocking is one of the most elegant ideas in TCP. Let me build it from scratch.

---

#### The problem self-clocking solves

Suppose TCP has a congestion window of 8 packets. It is allowed to have 8 packets in flight. The naive question is: **when exactly should the sender transmit each packet?**

One bad answer: blast all 8 packets immediately at the sender's NIC speed.

```
Sender NIC (1 Gb/s)          Bottleneck (1 Mb/s)         Receiver
  │
  ├──► pkt1 ──────────────►│
  ├──► pkt2 ──────────────►│ queue fills
  ├──► pkt3 ──────────────►│ queue fills
  ├──► pkt4 ──────────────►│ queue overflows → DROP
  ├──► pkt5 ──────────────►│ queue overflows → DROP
  ├──► pkt6 ──────────────►│ queue overflows → DROP
  ├──► pkt7 ──────────────►│ queue overflows → DROP
  └──► pkt8 ──────────────►│ queue overflows → DROP
```

The sender can blast 8 packets in microseconds. The bottleneck can only absorb them at 1 Mb/s. The result is massive queue overflow — exactly the congestion we are trying to prevent.

So there is a deeper question beneath cwnd: **even when you know how many packets to send, how do you pace them correctly?**

Self-clocking is the answer.

---

#### The core idea

> **Only send a new packet when an ACK comes back.**

That one rule, combined with how the bottleneck link works, produces perfect pacing automatically — with no explicit rate calculation needed.

---

#### Why the bottleneck spaces out packets

This is the physical mechanism that makes everything work. Follow one burst of packets through the network:

```
Step 1: Sender blasts packets back-to-back on the fast link

Sender ──(1 Gb/s)──► Router

  time ──►
  │pkt1│pkt2│pkt3│pkt4│   ← packets arrive at router nearly simultaneously
```

```
Step 2: Router forwards them one at a time on the slow bottleneck

Router ──(1 Mb/s)──► Receiver

  time ──►
  │pkt1│    │pkt2│    │pkt3│    │pkt4│   ← packets spaced out by bottleneck
       ↑gap↑      ↑gap↑
  
  Each gap = packet_size / bottleneck_rate
           = 1,500 bytes / 1 Mb/s = 12 ms
```

The bottleneck acts like a **metering valve**. Packets arrive in a burst, but they leave one at a time, spaced at the bottleneck rate. This spacing is called **inter-packet gap**.

```
Before bottleneck:   │p1│p2│p3│p4│  (bunched together)
After bottleneck:    │p1│──│p2│──│p3│──│p4│  (evenly spaced)
                          ↑    ↑    ↑
                         gaps created by bottleneck
```

---

#### How ACKs inherit the spacing

ACK packets travel back on the reverse path. Since ACKs are tiny (just headers, ~40 bytes), the reverse path rarely reorders or bunches them. They preserve the spacing created by the bottleneck:

```
Receiver sends ACKs immediately as packets arrive:

Receiver ──(fast return path)──► Sender

  time ──►
  │ACK1│──│ACK2│──│ACK3│──│ACK4│
        ↑        ↑        ↑
   same gaps as the forward packets
   (bottleneck spacing is preserved)
```

So ACKs arrive at the sender **spaced at the bottleneck rate** — even if the sender originally sent the packets in a burst.

---

#### The clocking mechanism

Now apply the rule: **send one new packet for each ACK received.**

```
Sender                Bottleneck              Receiver
  │                       │                      │
  ├──pkt1────────────────►│                      │
  ├──pkt2────────────────►│                      │
  ├──pkt3────────────────►│  (queued)            │
  ├──pkt4────────────────►│  (queued)            │
  │                       ├──pkt1───────────────►│
  │                       │                      ├──ACK1──────────►│
  │◄──ACK1────────────────────────────────────── │
  ├──pkt5────────────────►│   ← sent immediately on ACK1
  │                       ├──pkt2───────────────►│
  │                       │                      ├──ACK2──────────►│
  │◄──ACK2────────────────────────────────────── │
  ├──pkt6────────────────►│   ← sent immediately on ACK2
  │                       ├──pkt3───────────────►│
  ...
```

Look at what happens to pkt5, pkt6, pkt7...:

```
pkt5 is sent when ACK1 arrives.
ACK1 arrives spaced at the bottleneck rate.
Therefore pkt5 enters the network spaced at the bottleneck rate.

pkt6 is sent when ACK2 arrives.
ACK2 arrives spaced at the bottleneck rate.
Therefore pkt6 enters the network spaced at the bottleneck rate.
```

The sender is injecting new packets into the network at **exactly the rate the bottleneck is draining them.** No explicit rate calculation. No timer. The ACKs themselves are the clock.

```
Bottleneck drains at R bps
→ ACKs return at rate R
→ sender injects new packets at rate R
→ bottleneck always has packets to send
→ link fully utilized
→ queue never grows
```

---

#### The steady state picture

Once self-clocking is established, the network reaches a beautiful equilibrium:

```
                    ┌──────────────── pipe ────────────────┐
                    │                                       │
Sender          [pkt8][pkt7][pkt6][pkt5][pkt4][pkt3][pkt2][pkt1]        Receiver
  │◄──ACK──────────────────────────────────────────────────────────────── │
  │──pkt9──────────────────────────────────────────────────────────────►  │

For every packet that exits the pipe on the right (ACK comes back),
one new packet enters the pipe on the left.

The number of packets in the pipe stays constant = W.
The rate of packets through the bottleneck stays constant = R.
```

This is why it is called **self-clocking** — the network clocks itself. The ACKs from previously sent packets automatically regulate when new packets are sent. No external rate control needed.

---

#### What self-clocking actually prevents

Self-clocking solves two problems simultaneously:

##### Problem 1: Bursting into a congested pipe

```
Without self-clocking:
  Sender blasts W packets → burst hits bottleneck → queue spike → drops

With self-clocking:
  Sender sends one packet per ACK → packets enter at drain rate → no burst
```

##### Problem 2: Underutilizing the link

```
Without self-clocking (with a conservative timer):
  Sender waits for a fixed timer before sending more → link goes idle

With self-clocking:
  Sender sends immediately on each ACK → link always has work to do
```

Self-clocking achieves maximum utilization with minimum queueing — the ideal operating point.

---

#### What breaks self-clocking

Self-clocking depends on ACKs being spaced. Several things can disrupt this:

##### Delayed ACKs

Some TCP implementations delay ACKs (send one ACK per two packets, or wait up to 200ms). This bunches ACKs together, which bunches the sender's transmissions:

```
Delayed ACKs:
  │ACK1+ACK2│────────│ACK3+ACK4│
  
Sender response:
  │pkt5│pkt6│────────│pkt7│pkt8│  ← mini-bursts, not smooth flow
```

##### ACK compression

On asymmetric links (fast download, slow upload), the return path can become the bottleneck. ACKs bunch up there, destroying the spacing:

```
Fast forward path:  │p1│──│p2│──│p3│──│p4│  (nicely spaced)
Slow return path:   │ACK1ACK2ACK3ACK4│──────  (all bunched)

Sender sees burst of ACKs → sends burst of packets → defeats self-clocking
```

##### Timeouts

After a timeout, TCP retransmits and waits. When the ACK comes back, the self-clocking rhythm has been broken. TCP must re-establish it — which is one reason why slow start runs again after a timeout.

---

#### The deeper insight

Self-clocking is an instance of a general principle called **conservation of packets**:

> In steady state, the number of packets in the network should be constant. For every packet that leaves, exactly one enters.

```
packets in network = W  (the congestion window)

Packet leaves network → ACK arrives → sender injects new packet
                         ↑
                    this is the conservation event

W stays constant. Rate stays constant. Queue stays stable.
```

When W is set correctly (equal to the bandwidth-delay product), this conservation principle keeps the pipe exactly full without overflow.

---

#### Summary

```
┌──────────────────────────────────────────────────────────────┐
│                    How Self-Clocking Works                   │
│                                                              │
│  1. Sender transmits initial window (W packets)              │
│                                                              │
│  2. Bottleneck link spaces packets out at its drain rate R   │
│     (packets arrive bunched, leave evenly spaced)            │
│                                                              │
│  3. ACKs flow back preserving that spacing                   │
│     (ACKs arrive at sender spaced at rate R)                 │
│                                                              │
│  4. Sender transmits one new packet per ACK                  │
│     (new packets enter network at exactly rate R)            │
│                                                              │
│  5. Bottleneck always has work → link fully utilized         │
│     Queue never grows → no congestion                        │
│                                                              │
│  Result: sender automatically matches bottleneck rate        │
│          with no explicit rate measurement or timer          │
│          ACKs ARE the clock                                  │
│                                                              │
│  Breaks when: ACKs delayed, ACK compression, timeouts        │
└──────────────────────────────────────────────────────────────┘
```


The elegance is that the bottleneck itself — the very thing that causes congestion — becomes the mechanism that prevents it, by spacing out the ACKs that pace the sender.


---

## 13. TCP Reno: Fast Retransmit and Fast Recovery

### TCP Tahoe 
- On timeout or triple duplicate ack (implies lost packet)
  - Set threshold to congestion window/2
  - Set congestion window to 1
  - Retransmit missing segment (fast retransmit for triple duplicate ack)
  - Enter slow start state

TCP Tahoe had one weakness: on a triple duplicate ACK, it dropped the congestion window all the way to 1 and entered slow start. This is unnecessarily aggressive when only a single packet was lost.

### TCP Reno
- Same as Tahoe on timeout
- On triple duplicate ack
  - Set threshold to congestion window/2
  - Set congestion window to congestion window/2 (fast recovery)
  - Inflate congestion window size while in fast recovery (fast recovery)
  - Retransmit missing segment (fast retransmit)
  - Stay in congestion avoidance state
 
### TCP NewReno

Same as Tahoe/Reno on timeout
During fast recovery:
- Keep trace of last unacknowledgeed packet when entering fast recovery
- On every duplicate ack, inflate congestion window by maximum segment size
- When last packet acknowledged, return to congestion avoidance state, set cwnd back to value set when entering fast recovery
- Start sending out new packets while fast retransmit is in flight


### Three Mechanisms
- Fast retransmit (TCP Tahoe): 
  don’t wait for a timeout to retransmit a missing segment if you receive a triple duplicate acknowledgement 
  - Only drop back to slow start state on a timeout

- Fast recovery (TCP Reno): 
  halve the congestion window (don’t set it to 1) on triple duplicate acknowledgements

- Fast recovery (TCP Reno): 
  while in fast recovery state, inflate the congestion window as acknowledgements arrive, to keep data flowing
  - Each duplicate ack increases congestion window by 1
  - If the old window is c, then the new window is c/2
  - Receiving c acks will increase window size to 3c/2 -- can send c/2 new segments




#### Fast Retransmit (also in TCP Tahoe)


> If the sender receives **3 duplicate ACKs**, retransmit the missing segment **immediately** without waiting for a timeout.

Three duplicate ACKs are strong evidence that:
- The missing segment was lost
- Subsequent segments are arriving successfully (the receiver is sending acks for them)
- The network is not collapsed — no need for slow start

#### Fast Recovery (TCP Reno only)

On 3 duplicate ACKs:
- **Tahoe:** `cwnd = 1`, enter slow start
- **Reno:** `cwnd = cwnd / 2`, stay in congestion avoidance

Since `ssthresh = cwnd / 2` and `cwnd = ssthresh`, TCP Reno never exits congestion avoidance on a triple dup ACK. This is true AIMD.

##### Congestion Window Inflation

**Problem:** After fast retransmit, TCP must wait an entire RTT for the retransmitted segment's ACK before it can advance the send window and send new data. During this time, the network is idle.

**Solution:** For each duplicate ACK received while in fast recovery:
```
cwnd += 1 MSS
```

Since each duplicate ACK means a segment has left the network (the receiver is acknowledging something), TCP can safely send one new segment without increasing congestion.

**Net effect:** If the old window was `c`:
- After fast retransmit: `cwnd = c/2`
- After receiving `c` duplicate ACKs: `cwnd = c/2 + c = 3c/2`
- TCP can send `c/2` new segments during the recovery RTT

When the retransmission is finally ACKed, TCP deflates the window back to `c/2` and resumes normal congestion avoidance.

#### TCP Reno FSM

```
           ┌────────────────────────────────────────────┐
           │                                            │
           │   dup ack: cwnd += 1                       │
           ▼                                            │
  ┌─────────────────┐   triple dup ack         ┌───────────────────────┐
  │  Fast Recovery  │◄─────────── ─────────────┤  Congestion Avoidance │
  │                 │                          │  ack: cwnd+=MSS²/cwnd │
  │                 │  new ack:                └───────────────────────┘
  │                 │  cwnd = ssthresh                     ▲
  └─────────────────┘       │             cwnd > ssthresh  │
           │                └──────────────────────────────┘
           │ timeout                    │
           │                     ┌──────────────┐
           └───────────────────► │  Slow Start  │
                                 │  cwnd += MSS │
                                 └──────────────┘
                                       │ timeout
                                       │ cwnd=1, ssthresh=cwnd/2
                                       └── (self-loop)
```

#### TCP Reno vs TCP Tahoe comparison

| Event | TCP Tahoe | TCP Reno |
|---|---|---|
| Triple dup ACK | `cwnd = 1`, slow start | `cwnd /= 2`, stay in CA |
| Timeout | `cwnd = 1`, slow start | `cwnd = 1`, slow start |
| Fast retransmit | Yes | Yes |
| Fast recovery | No | Yes |
| Window inflation | No | Yes |





---

### Q1: Compare  TCP Tahoe vs TCP Reno vs TCP NewReno ?

Let me build this comparison carefully — not just listing features, but showing exactly why each version was needed and what problem it solved.

---

#### The evolution: each version fixes a specific weakness

```
Pre-Tahoe → Tahoe → Reno → New Reno
              ↑        ↑        ↑
         fixed       fixed    fixed
         collapse   slow     multiple
                    recovery  loss
```

---

#### TCP Tahoe (1988)

##### What it introduced

Tahoe was Van Jacobson's complete fix for congestion collapse. It introduced everything we have been studying:

```
- Congestion window (cwnd)
- Slow start
- Congestion avoidance
- ssthresh
- RTT estimation with variance
- Self-clocking
- Fast retransmit (on triple dup ACK)
```

##### How Tahoe responds to loss

Tahoe has **one response to all loss events** — whether timeout or triple dup ACK:

```
ssthresh = cwnd / 2
cwnd     = 1 MSS
→ enter slow start
```

##### Tahoe FSM

```
                    cwnd < ssthresh
                          │
        ┌─────────────────▼──────────────────┐
        │            Slow Start              │
        │      new ACK: cwnd += MSS          │
        └─────────────────┬──────────────────┘
                          │ cwnd ≥ ssthresh
                          ▼
        ┌────────────────────────────────────┐
        │        Congestion Avoidance        │
        │   new ACK: cwnd += MSS²/cwnd       │
        └────────────────────────────────────┘

  ⚡ timeout OR triple dup ACK (from either state):
       ssthresh = cwnd / 2
       cwnd = 1
       retransmit missing segment
       → enter Slow Start
```

##### Tahoe behavior over time

```
cwnd
│         ssthresh=8
│    /\       ↓
│   /  \    /────────────────●  ← triple dup ACK
│  /    \  /                  \
│ /      \/                    \  ssthresh=cwnd/2
│/ SS    CA                  SS \/CA
│                                 ────────────────
└──────────────────────────────────────────────► time
```

##### Tahoe's weakness

Tahoe treats triple dup ACK the same as timeout — it resets cwnd to 1. But these two events are very different:

```
Timeout:           The pipe went silent. ACKs stopped completely.
                   Something is seriously wrong.
                   Resetting cwnd to 1 makes sense.

Triple dup ACK:    The receiver is still getting packets — just not one specific one.
                   ACKs are still flowing (3+ of them just arrived!).
                   The network is NOT broken. One packet is missing.
                   Resetting cwnd to 1 is unnecessarily aggressive.
```

After a triple dup ACK, Tahoe drops cwnd to 1 and enters slow start. This wastes an entire slow start phase for what might be a single isolated loss:

```
Tahoe after triple dup ACK:

cwnd
│              drop to 1
│  ────────●     ↓
│           \  ● ← slow start restarts here
│            \/  ●
│                  ●
│                    ●─────── ← finally back to CA
│
└──────────────────────────────────────────────► time
     ↑ wasted RTTs in unnecessary slow start
```

This is what TCP Reno fixed.

---

#### TCP Reno (1990)

##### The key insight Reno adds

Triple dup ACK and timeout are different signals. They should get different responses:

```
Timeout          → catastrophic, reset to 1, slow start
Triple dup ACK   → minor, just one loss, stay in CA, halve cwnd
```

Reno adds a **third state: Fast Recovery.**

##### Reno's response to triple dup ACK

```
On triple dup ACK:
  ssthresh = cwnd / 2
  cwnd     = cwnd / 2    ← NOT reset to 1
  → enter Fast Recovery  ← NOT slow start
  → retransmit missing segment immediately (fast retransmit)

On timeout (same as Tahoe):
  ssthresh = cwnd / 2
  cwnd     = 1
  → enter Slow Start
```

##### Fast Recovery: what happens during it

While waiting for the retransmitted segment's ACK, the sender receives more duplicate ACKs. Each dup ACK proves another segment left the network. Reno inflates cwnd by 1 MSS per dup ACK:

```
On each duplicate ACK during Fast Recovery:
  cwnd += 1 MSS    (window inflation)
```

Why? Each dup ACK means one more packet was delivered successfully. The network has room for one more packet. So send one:


##### Fast Recovery Example

Good catch to question this. Let me trace through it very carefully because there are some subtle sequencing issues worth getting exactly right.

---

###### Setup: what we know before the loss

```
cwnd = 12 MSS      (old window size, call it c = 12)
ssthresh = some large value (not yet relevant)

Sender has sent pkts 1–12, all in flight.
Pkt 5 is dropped at the router.
Pkts 6, 7, 8, 9, 10, 11, 12 are delivered successfully.
```

---

###### What the receiver does

The receiver gets pkts 1, 2, 3, 4 — sends ACKs normally.
Then pkt 5 is missing. Pkts 6, 7, 8, 9... arrive out of order.

TCP receiver rule: when an out-of-order segment arrives, resend the last in-order ACK.

```
Receiver gets pkt 6  → sends dup ACK for pkt 4 (still waiting for pkt 5)
Receiver gets pkt 7  → sends dup ACK for pkt 4
Receiver gets pkt 8  → sends dup ACK for pkt 4
Receiver gets pkt 9  → sends dup ACK for pkt 4
...
```

Each dup ACK says: "I received something, but I am still waiting for pkt 5."

---

###### What the sender does: the precise sequence

- Phase 1: counting dup ACKs before fast recovery

```
dup ACK 1 arrives:  count = 1, cwnd unchanged = 12
dup ACK 2 arrives:  count = 2, cwnd unchanged = 12
dup ACK 3 arrives:  count = 3  ← THRESHOLD REACHED
```

On the **3rd dup ACK**, Reno triggers fast recovery:

```
ssthresh = cwnd / 2 = 12 / 2 = 6
cwnd     = ssthresh + 3 = 6 + 3 = 9
             ↑                ↑
        halved value    +3 for the 3 dup ACKs already received
                        (they each proved a packet left the network)

Retransmit pkt 5 immediately  ← fast retransmit
Enter Fast Recovery state
```

The `+3` is the first window inflation — the 3 dup ACKs that triggered fast recovery each represent a packet that left the network, so cwnd gets inflated by 3 MSS right away.

###### The core principle behind window inflation
Great question. Let me build the reasoning from first principles so the `+3` is not a magic number but an inevitable consequence of the logic.

---

- The core principle behind window inflation

Every duplicate ACK carries one piece of information:

> "I received another packet successfully. Something new left the network."

This is the key physical fact. A dup ACK is not just noise — it is proof that a packet was delivered. The receiver would not send a dup ACK unless it received an out-of-order segment.

```
Sender                              Receiver
  │── pkt 5 ──► DROPPED              │
  │── pkt 6 ──────────────────────►  │ got pkt 6 → sends dup ACK
  │── pkt 7 ──────────────────────►  │ got pkt 7 → sends dup ACK
  │── pkt 8 ──────────────────────►  │ got pkt 8 → sends dup ACK
  │
  │ Sender receives 3 dup ACKs.
  │ Each one proves: one packet left the network.
  │ Network now has room for one more packet each time.
```

---

- The conservation principle

Recall self-clocking and the conservation of packets:

```
Packets in network = cwnd (in steady state)

When a packet LEAVES the network:
  → network has room for one more
  → sender may inject one new packet

This is the foundation of window inflation.
```

Every dup ACK = one packet left = room for one more. Therefore:

```
Each dup ACK received → cwnd += 1 MSS
```

This is not a special rule invented for fast recovery. It is the **same conservation principle** that drives self-clocking, now applied explicitly during recovery.

---

- Where does the `+3` come from?

By the time fast recovery is triggered, **3 dup ACKs have already arrived.** Each one proved a packet left the network. But cwnd was not inflated for any of them yet — the sender was just counting.

So when the 3rd dup ACK arrives and fast recovery is triggered, the sender must account for all 3 dup ACKs at once:

```
dup ACK 1 arrived: proved pkt 6 left network → owes +1 MSS inflation (not yet applied)
dup ACK 2 arrived: proved pkt 7 left network → owes +1 MSS inflation (not yet applied)
dup ACK 3 arrived: proved pkt 8 left network → owes +1 MSS inflation (not yet applied)

At fast recovery entry: apply all 3 owed inflations at once → +3 MSS
```

This is why `cwnd = ssthresh + 3`:

```
ssthresh = cwnd / 2        (halve for multiplicative decrease)
cwnd     = ssthresh + 3    (inflate for the 3 dup ACKs already received)
         = c/2 + 3
```

The `+3` is **back-paying the debt** for 3 dup ACKs that each proved one packet left the network.

---

- Tracing the bytes in flight to verify this makes sense

Let us be precise. Before the loss:

```
cwnd = 12, pkts 1–12 in flight, pkt 5 dropped.
Pkts 1–4 already ACKed before loss detection.
Pkts 6, 7, 8 delivered → triggered the 3 dup ACKs.
```

At the moment of fast recovery entry:

```
What is actually in the network right now?
  pkts 9, 10, 11, 12   → still in flight (not yet delivered)
  pkt 5 retransmitted  → just injected into network
  
  Actual packets in network = 4 (pkts 9-12) + 1 (retransmit) = 5 packets

What does cwnd say?
  cwnd = ssthresh + 3 = 6 + 3 = 9 MSS

  Can sender inject more? Only if cwnd > bytes in flight.
  9 > 5 → yes, can send 4 more new packets immediately.
```

Wait — that seems like too many. Let me recount more carefully:

```
Pkts 1–4:   ACKed before loss. Not in flight.
Pkt 5:      lost, retransmitted. In flight again.
Pkts 6–8:   delivered (caused the 3 dup ACKs). Left network. NOT in flight.
Pkts 9–12:  still traveling. In flight.

Bytes in flight = pkt5(retransmit) + pkts 9,10,11,12 = 5 packets

cwnd = 9
cwnd - in_flight = 9 - 5 = 4 new packets may be sent.
```

Hmm, that gives 4, not the conservative behavior we want. Let me think about why this still makes sense:

```
Pkts 6, 7, 8 left the network (proved by 3 dup ACKs).
Network has room for 3 more → inflation +3 accounts for exactly this.

But pkts 9, 10, 11, 12 are still in flight = 4 packets.
Plus retransmit of pkt 5 = 1 packet.
Total still consuming network capacity = 5 packets.

cwnd was halved to 6 (ssthresh).
So we "want" only 6 packets in flight at steady state.
Currently 5 are in flight.
6 - 5 = 1 new packet can be sent.

But cwnd = 9, not 6. The extra 3 accounts for pkts 6,7,8
that LEFT the network but whose ACK credit we have received.

Net available slots = cwnd - in_flight = 9 - 5 = 4.

This seems too aggressive. The reason is that
in practice the sender accounts for bytes_in_flight
more carefully, and sends are gated by:

  can_send = (cwnd > bytes_in_flight)

As the sender sends new packets, bytes_in_flight grows,
so it cannot blindly send 4 at once.
```

The key practical point: cwnd inflation does not let the sender flood the network. It simply keeps the sender from going **idle** during the recovery RTT.

---

- The mental model that makes `+3` obvious

Think of cwnd as a **budget of slots** in the network. Each in-flight packet consumes one slot.

```
Before loss:   budget = 12 slots, 12 packets using them

After halving: budget = 6 slots  (we want only 6 in network)
               but 5 packets still using slots (pkts 5-retx, 9,10,11,12)
               
               Without inflation: can send only 1 new packet.

After +3:      budget = 9 slots
               5 packets using slots
               can send 4 new packets gradually (one per dup ACK received)
```

But wait — why should the budget be 9 and not 6? Because:

```
Pkts 6, 7, 8 ALREADY LEFT the network.
They freed up 3 slots.
Those freed slots are real — the network genuinely has room.
Inflation claims those 3 freed slots back into the budget.

budget = ssthresh (our target) + 3 (slots freed by pkts 6,7,8)
       = 6 + 3 = 9
```

---

- Why subsequent dup ACKs each add +1

After fast recovery entry, more dup ACKs arrive (from pkts 9, 10, 11, 12...):

```
dup ACK 4: pkt 9 delivered → left network → freed 1 slot → cwnd += 1 = 10
dup ACK 5: pkt 10 delivered → left network → freed 1 slot → cwnd += 1 = 11
dup ACK 6: pkt 11 delivered → left network → freed 1 slot → cwnd += 1 = 12
dup ACK 7: pkt 12 delivered → left network → freed 1 slot → cwnd += 1 = 13
```

Each one: a packet leaves, a slot opens, cwnd claims it. Exactly the conservation principle.

---

- Why cwnd deflates back to ssthresh on new ACK

When the retransmission ACK finally arrives:

```
New ACK acknowledges everything up through the recovered packet.
All the dup-ACK packets (6,7,8,9,10,11,12) are now properly ACKed.
Fast recovery is over.

The correct cwnd going forward = ssthresh = c/2.
(We halved because of the loss. That decision stands.)

So: cwnd = ssthresh   ← deflate, discard the inflation
```

The inflation was **temporary scaffolding** — it kept the pipe full during recovery, but it was never meant to persist. Once the loss is resolved, the permanent decision (halve the window) takes effect.

---

- Summary

```
┌──────────────────────────────────────────────────────────────┐
│  Why cwnd = ssthresh + 3 at fast recovery entry              │
│                                                              │
│  Core principle:                                             │
│    Each dup ACK proves one packet left the network.          │
│    Network has room for one more packet.                     │
│    Therefore: each dup ACK → cwnd += 1 MSS                  │
│                                                              │
│  By the time fast recovery triggers (3rd dup ACK):           │
│    3 dup ACKs have already arrived.                          │
│    3 packets have already left the network.                  │
│    3 slots are genuinely free in the network.               │
│    We owe 3 × (+1 MSS) inflation that was not yet applied.  │
│                                                              │
│  So at entry:                                                │
│    ssthresh = cwnd / 2    (permanent: halve for the loss)    │
│    cwnd = ssthresh + 3    (temporary: claim the 3 freed      │
│                            slots from pkts 6,7,8)            │
│                                                              │
│  Subsequent dup ACKs:                                        │
│    Each proves another packet left → cwnd += 1               │
│    Keeps sender from going idle during recovery RTT          │
│                                                              │
│  On new ACK:                                                 │
│    cwnd = ssthresh        (discard temporary inflation)      │
│    Recovery complete, resume congestion avoidance            │
└──────────────────────────────────────────────────────────────┘
```

The `+3` is not a hardcoded constant. It is the exact number of dup ACKs that arrived before fast recovery was triggered — each one representing a real freed slot in the network. If the threshold were 4 dup ACKs instead of 3, the entry inflation would be `+4` for exactly the same reason.




- Phase 2: continuing to receive dup ACKs in fast recovery

Now the sender is in fast recovery, waiting for the retransmitted pkt 5 to be acknowledged. More dup ACKs keep arriving from pkts 10, 11, 12...

```
dup ACK 4 arrives:  cwnd = 9 + 1 = 10
dup ACK 5 arrives:  cwnd = 10 + 1 = 11
dup ACK 6 arrives:  cwnd = 11 + 1 = 12
dup ACK 7 arrives:  cwnd = 12 + 1 = 13
...
```

Each dup ACK inflates cwnd by 1. This lets the sender transmit new packets to keep the pipe full while waiting for the retransmission ACK.

- Phase 3: retransmission ACK arrives (new ACK)

When pkt 5's retransmission is delivered, the receiver has pkts 1–12 complete. It sends a new ACK acknowledging everything up to pkt 12.

```
New ACK arrives (acknowledges pkt 12):
  cwnd = ssthresh = 6    ← deflate back to the halved value
  Exit Fast Recovery
  → Enter Congestion Avoidance
```

---

###### Why `cwnd = ssthresh + 3` at fast recovery entry?

This is the part most explanations gloss over. Let me make the physical reasoning explicit.

The 3 dup ACKs that triggered fast recovery each proved that a packet left the network:

```
dup ACK 1 caused by pkt 6 arriving at receiver → pkt 6 left the network
dup ACK 2 caused by pkt 7 arriving at receiver → pkt 7 left the network
dup ACK 3 caused by pkt 8 arriving at receiver → pkt 8 left the network
```

Three packets left the network. The network has room for three more. So we inflate cwnd by 3 immediately:

```
cwnd = ssthresh + 3
     = (c/2) + 3
```

This is not a special rule — it is the same per-dup-ACK inflation rule applied to all 3 triggering dup ACKs at once.

---

###### What cwnd inflation actually allows: can we send new packets?

Inflation only helps if the inflated cwnd exceeds the number of bytes currently in flight. Let me trace this carefully.

```
At time of 3rd dup ACK:
  Packets in flight = pkts 5 (retransmitted), 6, 7, 8, 9, 10, 11, 12
                    = 8 packets in flight
                    (pkt 5 original was lost but retransmit counts)

  cwnd = 9

  Can send if: cwnd > bytes in flight
  9 > 8 → YES, can send 1 new packet (pkt 13)
```

After each subsequent dup ACK:

```
dup ACK 4: bytes in flight still 8 (pkt 9 delivered = -1, but we just sent pkt 13 = +1)
           cwnd = 10 > 8 → can send pkt 14

dup ACK 5: cwnd = 11 > 8 → can send pkt 15

...
```

The inflation keeps cwnd just ahead of the in-flight count, allowing the sender to inject one new packet per dup ACK received. This keeps the pipe full while waiting for the retransmission ACK.

---

###### Net effect derivation: where does `c/2` new segments come from?

Starting state: cwnd = c = 12, one loss.

```
Packets already delivered when fast recovery ends:
  pkts 6–12 = 7 packets delivered (generated 7 dup ACKs after the 3 triggering ones... 
  actually let us count properly)

Old window had c = 12 packets in flight: pkts 1-12
pkt 5 lost.
pkts 1-4 already ACKed before the loss.
pkts 6-12 = 7 packets delivered (generated dup ACKs).
pkt 5 retransmitted.

New packets sent during fast recovery:
  One per dup ACK received after entry = 7 - 3 = 4... 

Let me count from first principles instead.
```

Actually the cleanest way to see the net effect:

```
Before fast recovery:   cwnd = c,    ssthresh = c/2
After fast recovery:    cwnd = c/2   (deflated on new ACK)

During fast recovery, the sender transmitted:
  - 1 retransmission (pkt 5)
  - approximately c/2 new packets (pkts 13, 14, ...)

The pipe was kept busy for the entire recovery RTT.
```

The "c/2 new segments" claim in the original note is an approximation. The precise number depends on exactly how many dup ACKs arrive, which depends on how many packets were in the original window beyond the lost one.

---

###### Complete corrected timeline

```
State:    cwnd=12, ssthresh=large, pkt 5 dropped

Sender                              Receiver
  │── pkt 1 ──────────────────────►│ ACK 1
  │── pkt 2 ──────────────────────►│ ACK 2
  │── pkt 3 ──────────────────────►│ ACK 3
  │── pkt 4 ──────────────────────►│ ACK 4
  │── pkt 5 ──────────────────────►│ DROPPED
  │── pkt 6 ──────────────────────►│ dup ACK 4  (1st)   cwnd=12 unchanged
  │── pkt 7 ──────────────────────►│ dup ACK 4  (2nd)   cwnd=12 unchanged
  │── pkt 8 ──────────────────────►│ dup ACK 4  (3rd) ← FAST RECOVERY TRIGGERED
  │                                 │               ssthresh=6, cwnd=6+3=9
  │── pkt 5 (retransmit) ─────────►│               fast retransmit
  │── pkt 9 ──────────────────────►│ dup ACK 4  (4th)   cwnd=10, send pkt 13
  │── pkt 13 (new) ───────────────►│ dup ACK 4  (5th)   cwnd=11, send pkt 14
  │── pkt 10 ──────────────────────►│ dup ACK 4  (6th)  cwnd=12, send pkt 15
  │── pkt 14 (new) ───────────────►│ dup ACK 4  (7th)   cwnd=13
  │── pkt 11 ──────────────────────►│
  │── pkt 12 ──────────────────────►│
  │                                 │ ← pkt 5 retransmit arrives
  │◄── NEW ACK (up to pkt 15) ─────│ cwnd = ssthresh = 6
  │                                 │ EXIT fast recovery → Congestion Avoidance
```

---




##### Reno FSM

```
                              dup ACK: cwnd += 1
                         ┌──────────────────────────┐
                         ▼                           │
              ┌─────────────────────┐                │
              │    Fast Recovery    │◄───── triple dup ACK
              │                     │       ssthresh = cwnd/2
              └──────────┬──────────┘       cwnd = ssthresh + 3
                         │ new ACK:
                         │ cwnd = ssthresh
                         │ → Congestion Avoidance
                         ▼
        ┌────────────────────────────────────┐
        │        Congestion Avoidance        │◄── cwnd ≥ ssthresh
        │   new ACK: cwnd += MSS²/cwnd       │
        └────────────────────────────────────┘
                         ↑
                    cwnd ≥ ssthresh
                         │
        ┌────────────────┴───────────────────┐
        │            Slow Start              │
        │      new ACK: cwnd += MSS          │
        └────────────────────────────────────┘

  ⚡ timeout (from any state):
       ssthresh = cwnd / 2
       cwnd = 1
       → Slow Start
```

##### Reno behavior vs Tahoe

```
cwnd        Tahoe                    cwnd        Reno
│    ────●                           │    ────●
│        │ reset to 1                │        │ halve only
│        ↓                           │        ↓
│      ● slow start                  │      ●─────── stays in CA
│    ●                               │
│  ●                                 │
│●──── finally back                  │
└───────────────────► time           └──────────────────► time
   ↑ many wasted RTTs                   ↑ much faster recovery
```

In steady state with only isolated losses and no timeouts, Reno achieves **true AIMD**:

```
Loss detected by triple dup ACK → cwnd halved  (multiplicative decrease)
ACKs received in CA              → cwnd +1/RTT  (additive increase)
```

##### Reno's weakness: multiple losses in one window

Reno works perfectly when **only one packet** is lost per window. But what if two packets are lost?

```
Sender window: pkt1 pkt2 pkt3 pkt4 pkt5 pkt6 pkt7 pkt8
                          ↑              ↑
                        LOST           LOST

Receiver gets: pkt1, pkt2, pkt4, pkt5, pkt6 (not pkt3 or pkt7)
Receiver sends: dup ACK for pkt2 (×3 when pkt4,5,6 arrive)

Reno detects: triple dup ACK for pkt2
Reno retransmits: pkt3
Reno enters fast recovery, inflates cwnd

But: pkt7 is also lost. Reno does NOT know this yet.
When pkt3's ACK arrives, it acknowledges everything up to pkt6.
Reno exits fast recovery: cwnd = ssthresh

Now pkt7's loss is detected — but only via timeout,
because there are no more packets after pkt8 to generate dup ACKs.

Timeout → cwnd = 1 → unnecessary slow start
```

The problem: Reno can only detect **one loss per RTT**. If multiple packets are lost, it needs multiple RTTs (and possibly a timeout) to discover each one. This causes cwnd to be halved multiple times unnecessarily.

This is what New Reno fixed.

---

#### TCP New Reno

##### The insight New Reno adds

When Reno exits fast recovery on a new ACK, it might be a **partial ACK** — acknowledging some but not all of the packets that were in flight when fast recovery started.

```
Reno:     any new ACK → exit fast recovery immediately
New Reno: partial ACK → stay in fast recovery, retransmit next missing packet
          full ACK    → exit fast recovery (all losses recovered)
```

##### What is a partial ACK?

```
Flight at time of loss: pkt3(lost), pkt4, pkt5, pkt6, pkt7(lost), pkt8

Fast retransmit: retransmit pkt3

ACK comes back acknowledging up to pkt6 → PARTIAL ACK
  (pkt7 is still missing — this ACK did not cover everything)

Reno:     sees new ACK → exits fast recovery → misses pkt7 loss
New Reno: sees partial ACK → stays in fast recovery → immediately retransmits pkt7
```

##### New Reno FSM addition

```
In Fast Recovery:
  partial new ACK:
    retransmit next missing segment   ← NEW RENO ADDITION
    cwnd -= bytes acknowledged + MSS  ← adjust window
    stay in Fast Recovery

  full new ACK:
    cwnd = ssthresh
    exit Fast Recovery → Congestion Avoidance

  dup ACK:
    cwnd += 1 MSS   (same as Reno)

  timeout:
    cwnd = 1, ssthresh = cwnd/2
    → Slow Start
```

##### New Reno behavior with multiple losses

```
Losses: pkt3 and pkt7 both dropped

Reno:                          New Reno:
  detect pkt3 loss               detect pkt3 loss
  fast retransmit pkt3           fast retransmit pkt3
  enter fast recovery            enter fast recovery
  receive partial ACK (to pkt6)  receive partial ACK (to pkt6)
  EXIT fast recovery ← wrong     STAY in fast recovery ← correct
  wait...                        fast retransmit pkt7 immediately
  timeout on pkt7                receive full ACK
  cwnd = 1                       exit fast recovery
  slow start                     cwnd = ssthresh
  ← many RTTs wasted             ← recovered in one extra RTT
```

---

#### Side-by-side comparison

```
┌───────────────────┬──────────────────┬──────────────────┬──────────────────┐
│                   │   TCP Tahoe      │   TCP Reno       │  TCP New Reno    │
│                   │   (1988)         │   (1990)         │  (1996)          │
├───────────────────┼──────────────────┼──────────────────┼──────────────────┤
│ Congestion window │ Yes              │ Yes              │ Yes              │
│ Slow start        │ Yes              │ Yes              │ Yes              │
│ Congestion avoid. │ Yes              │ Yes              │ Yes              │
│ RTT estimation    │ Yes              │ Yes              │ Yes              │
│ Self-clocking     │ Yes              │ Yes              │ Yes              │
├───────────────────┼──────────────────┼──────────────────┼──────────────────┤
│ Fast retransmit   │ Yes              │ Yes              │ Yes              │
│ Fast recovery     │ No               │ Yes              │ Yes (improved)   │
│ Window inflation  │ No               │ Yes              │ Yes              │
│ Partial ACK       │ No               │ No               │ Yes              │
│ handling          │                  │                  │                  │
├───────────────────┼──────────────────┼──────────────────┼──────────────────┤
│ On triple dup ACK │ cwnd=1           │ cwnd=cwnd/2      │ cwnd=cwnd/2      │
│                   │ → slow start     │ → fast recovery  │ → fast recovery  │
├───────────────────┼──────────────────┼──────────────────┼──────────────────┤
│ On timeout        │ cwnd=1           │ cwnd=1           │ cwnd=1           │
│                   │ → slow start     │ → slow start     │ → slow start     │
├───────────────────┼──────────────────┼──────────────────┼──────────────────┤
│ Steady state      │ Approximate AIMD │ True AIMD        │ True AIMD        │
│ behavior          │ (drops to SS     │ (halves on loss, │ (same as Reno    │
│                   │  on dup ACK)     │ stays in CA)     │ + handles multi- │
│                   │                  │                  │  loss correctly) │
├───────────────────┼──────────────────┼──────────────────┼──────────────────┤
│ Single loss       │ Slow             │ Fast             │ Fast             │
│ recovery          │ (slow start)     │ (fast recovery)  │ (fast recovery)  │
├───────────────────┼──────────────────┼──────────────────┼──────────────────┤
│ Multiple loss     │ Very slow        │ Slow             │ Fast             │
│ recovery          │ (slow start      │ (timeout needed  │ (stays in fast   │
│                   │  per loss)       │ for 2nd loss)    │  recovery)       │
└───────────────────┴──────────────────┴──────────────────┴──────────────────┘
```

---

#### The progression in one picture

```
cwnd      Single loss scenario:

Tahoe:
│    ────●                        ← triple dup ACK
│        ↓ cwnd=1
│      ●
│    ●     ●─────────────
│  ●           ↑ finally back (many RTTs)
└───────────────────────────────► time

Reno:
│    ────●                        ← triple dup ACK
│        ↓ cwnd/2 only
│      ●─────────────────
│          ↑ back in 1 RTT
└───────────────────────────────► time

New Reno (single loss — same as Reno):
│    ────●                        ← triple dup ACK
│        ↓ cwnd/2 only
│      ●─────────────────
└───────────────────────────────► time


cwnd      Multiple loss scenario:

Reno:
│    ────●  ← 1st loss detected   ← 2nd loss causes timeout
│        ↓                              ↓
│      ●──●─partial ACK                cwnd=1
│               exits FR early          ●
│                                     ●   ← slow start again
└───────────────────────────────► time

New Reno:
│    ────●  ← 1st loss detected
│        ↓
│      ●──●─partial ACK → stays in FR
│               retransmits 2nd loss immediately
│               ●──────────────
│                   ↑ recovered in one extra RTT
└───────────────────────────────► time
```

---

#### Which one does the Internet actually use?

```
Modern Linux/macOS/Windows: TCP CUBIC (default) or BBR
                            but with New Reno as the fallback
                            and fast retransmit/recovery from Reno at the core

CS144 context:              Tahoe and Reno are what you study
                            New Reno is the RFC standard (RFC 6582)
                            CUBIC/BBR extend beyond what CS144 covers
```

The key insight across all three versions: **each generation got better at distinguishing signal from noise** — telling apart "one packet lost" from "network collapsed," and "one loss" from "multiple losses." The more precisely TCP can diagnose what went wrong, the less aggressively it needs to back off.


### Q2: What is a partial ACK?

Great question. Let me build this carefully from what a normal ACK means, then show exactly what makes a partial ACK different.

---

#### Background: how TCP ACKs work

TCP uses **cumulative ACKs**. An ACK number means:

> "I have received everything up to and including byte N. Send me byte N+1 next."

```
Sender sends:  pkt1, pkt2, pkt3, pkt4, pkt5
Receiver gets: pkt1, pkt2, pkt3, pkt4, pkt5

ACK says: "received everything up to pkt5" → ACK = 5+1 = next expected
```

Crucially, one ACK can acknowledge many packets at once:

```
Sender sends:  pkt1, pkt2, pkt3, pkt4, pkt5
Receiver gets: all five, sends one ACK for pkt5
→ that single ACK implicitly acknowledges pkt1, pkt2, pkt3, pkt4 too
```

---

#### Setting up the scenario

```
cwnd = 8 packets.
Sender has sent pkts 1–8, all in flight.
Two packets are lost: pkt3 and pkt7.

Timeline of what receiver gets:
  pkt1 → delivered ✓
  pkt2 → delivered ✓
  pkt3 → DROPPED ✗
  pkt4 → delivered ✓  (but out of order — waiting for pkt3)
  pkt5 → delivered ✓  (out of order)
  pkt6 → delivered ✓  (out of order)
  pkt7 → DROPPED ✗
  pkt8 → delivered ✓  (out of order)
```

What does the receiver do?

```
Gets pkt1 → ACK 1 (normal)
Gets pkt2 → ACK 2 (normal)
Gets pkt3 → MISSING
Gets pkt4 → dup ACK 2 (have pkt4 buffered, still waiting for pkt3)
Gets pkt5 → dup ACK 2 (have pkt4,5 buffered, still waiting for pkt3)
Gets pkt6 → dup ACK 2 (have pkt4,5,6 buffered, still waiting for pkt3)
Gets pkt7 → MISSING
Gets pkt8 → dup ACK 2 (have pkt4,5,6,8 buffered, still waiting for pkt3)
```

After 3 dup ACKs, sender triggers fast recovery and retransmits pkt3.

---

#### What happens when pkt3 retransmission arrives

The receiver now has in its buffer: pkt4, pkt5, pkt6, pkt8 (pkt7 still missing).

```
Receiver gets retransmitted pkt3:
  Now has: pkt1, pkt2, pkt3, pkt4, pkt5, pkt6 — all contiguous!
  pkt7 still missing.
  pkt8 buffered but cannot be delivered yet.

Receiver sends ACK: "received everything up to pkt6, send me pkt7"
→ ACK number = 7
```

This ACK acknowledges pkts 1–6. The sender sees a **new ACK** (higher than any previous ACK number).

---

#### Full ACK vs Partial ACK

Here is the critical distinction:

```
At the moment fast recovery started, what was in flight?
  pkts 1–8  (the full window of 8 packets)

Specifically, the packets that were unacknowledged when fast recovery began:
  pkt3 (lost), pkt4, pkt5, pkt6, pkt7 (lost), pkt8

The ACK that came back acknowledges up to pkt6.

Does this ACK cover EVERYTHING that was in flight? 
  NO — pkt7 and pkt8 are still unresolved.

→ This is a PARTIAL ACK.
```

Formally:

```
FULL ACK:    new ACK number ≥ highest sequence number in flight
             when fast recovery started
             → ALL losses have been recovered
             → safe to exit fast recovery

PARTIAL ACK: new ACK number < highest sequence number in flight
             when fast recovery started
             → some packets are still missing or unresolved
             → at least one more loss may exist
             → NOT safe to exit fast recovery yet
```

In the example:

```
Highest pkt in flight when fast recovery started = pkt8

ACK came back for pkt6.
  pkt6 < pkt8 → PARTIAL ACK

ACK came back for pkt8 (or higher) would be:
  pkt8 ≥ pkt8 → FULL ACK
```

---

#### What Reno does (wrong behavior)

Reno only knows one rule: **any new ACK = exit fast recovery**.

```
Reno sees ACK for pkt6:
  "This is a new ACK — higher than before. Recovery must be done."
  cwnd = ssthresh
  EXIT fast recovery
  → resume congestion avoidance

But pkt7 is still missing!
No more packets after pkt8 are in flight to generate dup ACKs for pkt7.
Sender sends new packets (pkt9, pkt10...) but pkt7 is still undelivered.

Eventually: RTO fires for pkt7.
  cwnd = 1
  → slow start
  → many RTTs wasted
```

Reno cannot distinguish "partial recovery" from "full recovery". It exits too early.

---

#### What New Reno does (correct behavior)

New Reno checks: is this ACK a partial ACK or a full ACK?

```
New Reno sees ACK for pkt6:
  Highest in-flight pkt when fast recovery started = pkt8
  ACK for pkt6 < pkt8 → PARTIAL ACK

  Response:
    Do NOT exit fast recovery
    Retransmit the next missing packet immediately (pkt7)
    Adjust cwnd:
      cwnd -= bytes acknowledged  (deflate for what left)
      cwnd += 1 MSS               (inflate for the retransmission)
    Stay in fast recovery
    Wait for next ACK
```

Now the retransmitted pkt7 arrives at the receiver:

```
Receiver now has: pkt1–pkt8 complete.
Sends ACK for pkt8.

New Reno sees ACK for pkt8:
  Highest in-flight pkt when fast recovery started = pkt8
  ACK for pkt8 ≥ pkt8 → FULL ACK

  Response:
    EXIT fast recovery
    cwnd = ssthresh
    → resume congestion avoidance
```

---

#### Side by side timeline

```
Event                    Reno                    New Reno
─────────────────────    ──────────────────────  ──────────────────────
pkt3, pkt7 dropped       (not yet known)         (not yet known)
3 dup ACKs received      fast retransmit pkt3    fast retransmit pkt3
                         enter fast recovery     enter fast recovery
pkt3 retransmit arrives  receiver sends ACK6     receiver sends ACK6
ACK6 received            "new ACK → exit FR"     "ACK6 < pkt8 → partial"
                         cwnd = ssthresh         retransmit pkt7
                         resume CA               stay in fast recovery
pkt7 loss undetected     waiting...              pkt7 retransmit arrives
                         ...                     receiver sends ACK8
timeout fires on pkt7    cwnd = 1                ACK8 received
                         slow start              "ACK8 ≥ pkt8 → full ACK"
                         many RTTs wasted        exit fast recovery
                                                 cwnd = ssthresh
                                                 resume CA
                                                 recovered in ~1 extra RTT
```

---

#### The key insight in one sentence

```
┌──────────────────────────────────────────────────────────────────┐
│  A partial ACK is a new ACK that advances the window forward     │
│  but does not reach the highest sequence number that was         │
│  in flight when fast recovery began.                             │
│                                                                  │
│  It means: one loss was recovered, but more losses may remain.   │
│                                                                  │
│  Reno:     cannot tell the difference → exits too early          │
│  New Reno: detects partial ACK → stays in recovery → retransmits │
│            next missing segment immediately instead of waiting   │
│            for a timeout                                         │
└──────────────────────────────────────────────────────────────────┘
```

The partial ACK is New Reno's mechanism for handling **multiple losses within a single window** — the exact scenario where Reno degrades to timeout-driven recovery and loses many RTTs unnecessarily.



## 14. Why AIMD Converges to Fairness: The Chiu-Jain Plot

- Service Provider: maximize link utilization
- User: I get my fair share
- Want network to converge to a state where everyone gets 1/N
- Avoid congestion collapse


Consider two flows A and B sharing a bottleneck link of capacity `C`. Plot their rates as a point `(rA, rB)`:

```
rB
│
│     Fair line: rA = rB
│    ╱
│   ╱  Efficient line: rA + rB = C
│  ╱           ╲
│ ╱    overload ╲
│╱               ╲
│    underload    ╲
└──────────────────── rA
```

The ideal operating point is where both lines intersect: `rA = rB = C/2`.

### How AIMD moves the operating point

**Additive increase** moves the point at a 45° angle toward the efficient line (both flows increase equally).

**Multiplicative decrease** moves the point toward the origin along the line from the current point to the origin.

```
rB
│            ╱ Fair
│           ╱
│    t2    ╱  t4
│     ●   ╱   ●
│    ╱   ╱t3 ╱
│  t1   ●   ╱
│   ●       ╱ Efficient
└───────────────── rA

Each step:
  AI: move diagonally (both increase)
  MD: move toward origin
```

**Result:** The operating point spirals toward the intersection of the fair line and the efficient line — regardless of the starting point. AIMD is self-stabilizing and converges to fair, efficient sharing.

---

### Q1: Why AIMD Converges to Fairness? What is The Chiu-Jain Plot and how to interpret the plot?

Great question. Let me build this completely from scratch — the geometry, the intuition, and exactly how to read the plot.

---

#### The question AIMD needs to answer

Suppose two flows A and B share a bottleneck link of capacity C. Neither flow knows what the other is doing. There is no central coordinator. Each runs AIMD independently.

The question is:

```
Will they converge to a fair allocation?
Or will one flow permanently dominate the other?
```

The Chiu-Jain plot answers this geometrically.

---

#### Setting up the 2D space

Plot the two flow rates as a single point:

```
rB (rate of flow B)
▲
│
│         ●  ← current allocation (rA, rB)
│
│
└──────────────────────────────► rA (rate of flow A)
```

Every possible allocation of rates between A and B is one point in this 2D space. As AIMD runs, the point moves around. We want to understand where it ends up.

---

#### Two lines that define the ideal operating point

##### Line 1: The efficiency line

The bottleneck link has capacity C. We want to use it fully:

```
rA + rB = C
```

This is a straight line with slope -1, cutting the axes at (C, 0) and (0, C):

```
rB
▲
C│╲
 │  ╲   rA + rB = C
 │    ╲  (efficiency line)
 │      ╲
 │        ╲
 └──────────╲──────────────► rA
             C
```

Points **below** this line: underutilizing the link — wasting capacity.
Points **above** this line: overloading the link — causing congestion and drops.

##### Line 2: The fairness line

Equal allocation means both flows get the same rate:

```
rA = rB
```

This is the diagonal at 45°:

```
rB
▲
│          ╱
│        ╱   rA = rB
│      ╱     (fairness line)
│    ╱
│  ╱
└╱──────────────────────────► rA
```

Points **above** this line: flow B gets more than A.
Points **below** this line: flow A gets more than B.

##### The ideal point: intersection of both lines

```
rA + rB = C   AND   rA = rB
→ 2rA = C
→ rA = rB = C/2
```

```
rB
▲
C│╲
 │  ╲
C/2├────X  ← ideal point (C/2, C/2)
 │  ╱ ╲╲     fair AND efficient
 │╱    ╲ ╲
 └──────╲──╲─────────────► rA
        C/2  C
```

Any allocation that is both fair and efficient must be at this point X.

---

#### The full Chiu-Jain plot with both lines

```
rB
▲
C│╲         ╱
 │  ╲     ╱
 │    ╲  ╱← fairness line (rA=rB)
 │ over ╲╱
 │  load ╱╲
 │     ╱    ╲← efficiency line (rA+rB=C)
C/2  ╱  ✓    ╲
 │ ╱  ideal   ╲
 │╱    region   ╲
 └──────────────────────────► rA
              C/2   C

Regions:
  Above efficiency line:   overload (rA+rB > C) → drops happening
  Below efficiency line:   underload (rA+rB < C) → wasting capacity
  Above fairness line:     B gets more than A
  Below fairness line:     A gets more than B
  On both lines:           ideal point X = (C/2, C/2)
```

---

#### How AIMD moves the point: two geometric operations

AIMD has two operations. Each moves the point in a specific geometric direction.

##### Operation 1: Additive Increase (AI)

Both flows increase their rate by the same fixed amount per RTT (+1 MSS).

```
rA → rA + δ
rB → rB + δ

where δ = 1 MSS per RTT (same for both flows)
```

This moves the point **diagonally at 45°** — equally in both directions:

```
rB
▲
│            ●  ← new point (moved up-right at 45°)
│          ↗
│        ●  ← old point
│      ↗
│    ●
│
└──────────────────────────────► rA

Direction: (δ, δ) — same increase for both flows
Slope of movement: Δrb/Δra = δ/δ = 1  → 45° diagonal
```

Key property: the diagonal direction is **parallel to the fairness line** (rA = rB also has slope 1). So additive increase moves the point **toward the efficiency line** without changing how fair the allocation is.

Wait — that is not quite right. Let me be more precise:

```
If current point is BELOW fairness line (rA > rB):
  Adding δ to both: new point is still rA+δ vs rB+δ
  Difference: (rA+δ) - (rB+δ) = rA - rB  (UNCHANGED)
  → AI does not change the absolute gap between flows

But relative fairness improves:
  Old ratio: rA/rB
  New ratio: (rA+δ)/(rB+δ) — closer to 1 if rA > rB
  → AI moves ratio toward fairness even if gap stays same
```

More importantly, AI moves the point toward the efficiency line:

```
rA + rB → (rA+δ) + (rB+δ) = rA + rB + 2δ

Total rate increases. Point moves toward efficiency line.
```

##### Operation 2: Multiplicative Decrease (MD)

When a drop occurs, both flows halve their rate:

```
rA → rA / 2
rB → rB / 2
```

This moves the point **along a line toward the origin**:

```
rB
▲
│      ●  ← old point (rA, rB)
│    ↙
│  ●  ← new point (rA/2, rB/2)
│↙
origin

Direction: straight toward (0,0)
The point moves along the line connecting origin to current point.
```

Key property: multiplication by 1/2 scales both coordinates equally. The new point lies on the **same ray from the origin** as the old point.

```
Old point:  (rA, rB)         → angle from x-axis = arctan(rB/rA)
New point:  (rA/2, rB/2)     → angle from x-axis = arctan(rB/rA)  SAME ANGLE

The ratio rB/rA is PRESERVED by multiplicative decrease.
```

This means MD moves the point closer to the origin along the same ray — it does not change the fairness ratio, just reduces the total rate.

---

#### Why these two operations together converge to fairness

Now watch what happens when you combine AI and MD repeatedly:

##### Case 1: Starting from an unfair point below the fairness line (A gets more)

```
rB
▲                         ╱ fair
│                       ╱
│              t4      ╱
│               ●    ╱
│             ↗  ╲ ╱
│    t3      ╱   ╱╲
│     ●    ╱   ╱    ╲ efficient
│    ╱ ╲ ╱  ╱
│  ╱   ╱╲╱
│t1   t2
│  ●
└──────────────────────────────► rA
  ↑
  start: A has much more than B (below fairness line)
```

Step by step:

```
t1: Start at unfair point. rA >> rB. Below fairness line.

AI phase: move diagonally up-right toward efficiency line.
  Both rates increase by δ each RTT.
  Point approaches efficiency line.

t2: Hit efficiency line (overload). Drop occurs.

MD phase: both halve → move toward origin along same ray.
  Ratio rB/rA preserved. Still below fairness line.
  But NOW closer to fairness line than before!

Why closer to fairness line after MD?
  The ray from origin to t2 has slope rB/rA < 1 (unfair, below diagonal).
  After MD: same slope but shorter.
  The new point t3 is closer to the origin AND
  the fairness line is at 45°...

  Actually: let us think more carefully.
```

##### The key geometric insight

Let me draw why each AI+MD cycle moves the point closer to the fairness line:

```
Before AI+MD cycle:
  Point P = (rA, rB)  where rA > rB (unfair, A dominates)

After AI phase (move to efficiency line):
  Point Q = (rA + α, rB + α)  for some α
  Note: Q is on or near the efficiency line

After MD phase (halve both):
  Point P' = (Q.x / 2, Q.y / 2)
           = ((rA + α)/2, (rB + α)/2)

Now compare fairness:
  Old gap:  rA - rB
  New gap:  (rA + α)/2 - (rB + α)/2
           = (rA - rB) / 2

The gap is HALVED each cycle!
```

This is the mathematical proof. Each complete AI+MD cycle halves the unfairness gap between the two flows. After many cycles:

```
Cycle 0:  gap = rA - rB = G
Cycle 1:  gap = G/2
Cycle 2:  gap = G/4
Cycle 3:  gap = G/8
...
Cycle n:  gap = G/2^n  → 0 as n → ∞
```

The allocation converges to `rA = rB = C/2` regardless of where it started.

---

#### Reading the full Chiu-Jain plot

Now let us trace through the complete plot with all the pieces:

```
rB
▲                    ╱ fairness line (rA=rB)
│            t4    ╱
│             ●──────────────────── efficiency line
│           ↗  ╲                  (rA+rB=C)
│         ╱    ╲
│    t3  ╱     ╲← MD: move toward origin
│     ●╱        ●t5
│    ╱↑           ╲
│  ╱ AI:           ╲
│╱  45° diagonal    ●t6──────────── efficiency line
│     t2●            ╲
│     ↗  ╲            ╲
│   ╱     ╲← MD        ╲
│t1●                    ●t7
│                          ╲
└──────────────────────────────────────────► rA
↑
start: very unfair (A gets almost everything)
```

Reading each transition:

```
t1 → t2:  AI phase. Both flows increase. 45° diagonal movement.
           Total rate (rA+rB) increases. Moving toward efficiency line.

t2:        Efficiency line hit. Overload. Drop occurs.

t2 → t3:  MD phase. Both halve. Move toward origin along ray from origin.
           Total rate decreases. Point moves below efficiency line.
           Gap (rA-rB) is halved.

t3 → t4:  AI phase again. 45° diagonal. Moving back toward efficiency line.
           Now starting from a fairer point than t1.

t4:        Efficiency line hit again. Drop.

t4 → t5:  MD phase. Halve. Move toward origin.
           Gap halved again.

...and so on. Each cycle:
  AI: move diagonally toward efficiency line
  MD: move toward origin, halving the fairness gap

The point spirals toward (C/2, C/2) — the ideal point.
```

---

#### Why AI moves at 45° and MD moves toward origin

These are the two geometric facts that make everything work. Let me state them cleanly:

```
AIMD operation    Mathematical effect      Geometric direction
──────────────    ──────────────────────   ────────────────────────────────
Additive          rA += δ                  45° diagonal (up and right)
Increase          rB += δ                  parallel to fairness line

Multiplicative    rA *= 1/2               Toward origin (along current ray)
Decrease          rB *= 1/2               preserves angle from x-axis
```

If MD also moved at 45° (additive decrease), the point would never get closer to the fairness line:

```
Additive decrease:  rA -= δ, rB -= δ
  New point: (rA-δ, rB-δ)
  New gap: (rA-δ) - (rB-δ) = rA - rB  (UNCHANGED)
  → never converges to fairness
```

This is the mathematical reason AIMD uses multiplicative decrease and not additive decrease. The multiplicative factor changes the direction of the decrease step, which is what causes convergence to fairness.

---

#### What happens with different starting points

The beauty of AIMD is that it converges from **any** starting point:

```
Starting above fairness line (B dominates):
  Same analysis, mirrored.
  MD halves the gap each cycle.
  Converges to (C/2, C/2) from above.

Starting on fairness line but below efficiency line:
  AI moves diagonally up → hits efficiency line exactly at (C/2, C/2).
  MD halves both → moves to (C/4, C/4).
  AI again → back to (C/2, C/2).
  Oscillates symmetrically around the ideal point.
  Already fair, stays fair.

Starting at origin:
  Slow start first (not pure AIMD), then AIMD takes over.
  Same convergence.
```

---

#### Summary

```
┌────────────────────────────────────────────────────────────────┐
│              The Chiu-Jain Plot: complete picture              │
│                                                                │
│  Axes:     rA and rB — the rates of two competing flows        │
│                                                                │
│  Efficiency line: rA + rB = C                                  │
│    Above it: overload, drops happening                         │
│    Below it: underutilization, wasting capacity                │
│                                                                │
│  Fairness line: rA = rB (45° diagonal)                         │
│    Above it: B gets more                                       │
│    Below it: A gets more                                       │
│                                                                │
│  Ideal point: intersection = (C/2, C/2)                        │
│    Both fair and efficient                                      │
│                                                                │
│  AI step: moves 45° diagonal toward efficiency line            │
│    Reduces underutilization                                    │
│    Does not worsen fairness gap in absolute terms              │
│                                                                │
│  MD step: moves toward origin along current ray                │
│    Halves the fairness gap (rA - rB) each cycle               │
│    This is why multiplicative (not additive) decrease          │
│    is essential for convergence to fairness                    │
│                                                                │
│  Combined: point spirals toward (C/2, C/2)                     │
│    From any starting point                                     │
│    Without any coordination between flows                      │
│    Purely from local AIMD decisions at each endpoint           │
└────────────────────────────────────────────────────────────────┘
```

The Chiu-Jain plot makes visible what would otherwise be a non-obvious mathematical property: two flows running independent AIMD, reacting only to their own packet drops, naturally converge to equal sharing of a bottleneck link.



## 15. Worked Example: AIMD Sawtooth Calculations

**Scenario:** Alice streams HD video at 10 Mb/s from San Francisco. All packets are 250 bytes (= 2,000 bits). Minimum ping time = 5 ms. Buffer is perfectly sized to never go empty.

### Q1: What is the minimum (trough) AIMD window?

The minimum RTT of 5 ms occurs when the buffer is empty but the bottleneck link is full.

```
Bytes in flight at minimum RTT = RTT × rate = 5 ms × 10 Mb/s = 50,000 bits = 6,250 bytes
```

**Answer: 6,250 bytes**

### Q2: What is the maximum (peak) AIMD window?

When both the buffer and link are full, RTT doubles to 10 ms.

```
Bytes in flight at maximum RTT = 10 ms × 10 Mb/s = 100,000 bits = 12,500 bytes
(6,250 bytes in flight on the link + 6,250 bytes in the buffer)
```

**Answer: 12,500 bytes**

### Q3: How big is the router buffer?

The buffer holds exactly the minimum RTT × rate = 50,000 bits.

**Answer: 6,250 bytes**

### Q4: After a drop, how long to reach the maximum window again?

Each packet is 2,000 bits. Window increases by one packet (2,000 bits) per RTT.

```
Distance to climb = 12,500 - 6,250 = 6,250 bytes = 50,000 bits
RTTs needed       = 50,000 / 2,000 = 25 RTTs
Average RTT       = (5 ms + 10 ms) / 2 = 7.5 ms
Time              = 25 × 7.5 ms = 187.5 ms
```

**Answer: 187.5 ms**

### Q5: If the server were in Australia (RTT = 250 ms), how big should the buffer be?

```
Buffer = min RTT × rate = 250 ms × 10 Mb/s = 2,500,000 bits ≈ 298 KB
```

**Answer: ~298 KB** (note: 50× larger buffer for 50× larger RTT)

### Q6: After a drop with the Australia server, how long to recover?

```
Buffer to fill     = 2,500,000 bits
Packets needed     = 2,500,000 / 2,000 = 1,250 RTTs
Average RTT        = (250 ms + 500 ms) / 2 = 375 ms
Time               = 1,250 × 375 ms = 468,750 ms ≈ 468 seconds ≈ 8 minutes
```

**Answer: ~8 minutes** — this illustrates how severely AIMD penalizes high-RTT connections.

---

## 16. Reading an RFC

### Q1: What is RFC document in newwork?
Great question. Let me build this from the ground up.

---

#### What RFC stands for

**RFC = Request for Comments**

The name sounds informal — and that is intentional. When the first RFC was written in 1969, the author Steve Crocker wanted to emphasize that these were discussion documents, not authoritative commands:

> "Mindful that our group was informal, junior and unchartered, I wanted to emphasize these notes were the beginning of a dialog and not an assertion of control."

The name stuck even as RFCs became the formal standards that define how the Internet works.

---

#### What an RFC actually is

An RFC is a **formal technical document** that defines, specifies, or discusses some aspect of Internet protocols and systems. RFCs are published by the **IETF** (Internet Engineering Task Force) — the main standards body for Internet protocols.

Think of RFCs as the **source of truth** for how Internet protocols must behave:

```
You want to know exactly how TCP works?
→ Read RFC 793 (original TCP specification)
→ Read RFC 5681 (TCP congestion control)

You want to know exactly how HTTP/1.1 works?
→ Read RFC 7230–7235

You want to know exactly how TLS 1.3 works?
→ Read RFC 8446
```

If your implementation disagrees with the RFC, your implementation is wrong — not the RFC.

---

#### A brief history

```
Year   Event
────   ──────────────────────────────────────────────────────
1969   RFC 1 published: "Host Software" by Steve Crocker
       First description of ARPANET host protocols

1973   RFC 454: File Transfer Protocol (FTP) specified

1974   RFC 675: First TCP specification (Cerf and Kahn)

1981   RFC 791: IP specification (still the foundation of IPv4)
       RFC 793: TCP specification (still the foundation of TCP)

1983   ARPANET switches to TCP/IP on January 1

1988   RFC 1122: Requirements for Internet Hosts
       RFC 1058: RIP routing protocol

1997   RFC 2119: Defines the terminology MUST/SHOULD/MAY
       (now used in almost every RFC)

2011   RFC 6298: TCP retransmission timer computation
       RFC 5681: TCP congestion control (covers Tahoe/Reno/NewReno)

Today  Over 9,000 RFCs published and counting
```

---

#### Kinds of RFCs

Not all RFCs carry the same weight. They are categorized by type:

```
┌─────────────────────┬──────────────────────────────────────────────────┐
│ Type                │ Meaning                                          │
├─────────────────────┼──────────────────────────────────────────────────┤
│ Standards Track     │ Intended to become an Internet Standard          │
│                     │ Goes through: Proposed Standard →                │
│                     │              Draft Standard →                    │
│                     │              Internet Standard                   │
├─────────────────────┼──────────────────────────────────────────────────┤
│ Informational       │ Provides useful information but not a standard   │
│                     │ e.g. tutorials, surveys, historical documents    │
├─────────────────────┼──────────────────────────────────────────────────┤
│ Experimental        │ Describes experimental work not ready            │
│                     │ for standardization                              │
├─────────────────────┼──────────────────────────────────────────────────┤
│ Best Current        │ Documents current best practices                 │
│ Practice (BCP)      │ e.g. security recommendations,                   │
│                     │ operational guidelines                           │
├─────────────────────┼──────────────────────────────────────────────────┤
│ Historic            │ Obsolete — superseded by a newer RFC             │
│                     │ or abandoned                                     │
└─────────────────────┴──────────────────────────────────────────────────┘
```

---

#### How an RFC gets published: the process

```
Step 1: Individual Draft
  Anyone can write a draft.
  Named: draft-<author>-<topic>-<version>
  Example: draft-jacobson-tcpm-congestion-00

Step 2: Working Group Draft
  A relevant IETF working group adopts the draft.
  Named: draft-ietf-<workinggroup>-<topic>-<version>
  Example: draft-ietf-tcpm-cubic-00
  Goes through multiple revisions.

Step 3: Working Group Last Call
  Working group members review and comment.
  Must reach rough consensus (not unanimous vote).

Step 4: IETF Last Call
  Broader IETF community reviews for 2–4 weeks.
  Anyone can comment.

Step 5: IESG Review
  Internet Engineering Steering Group formally reviews.
  Can approve, request changes, or reject.

Step 6: RFC Editor
  Document is edited for clarity and consistency.
  Assigned an RFC number.
  Published permanently.

Total time: months to years, depending on complexity.
```

---

#### The critical terminology: MUST, SHOULD, MAY

RFC 2119 defines precise meanings for key words used in all RFCs. These are not casual English — they have exact technical meanings:

```
Word                Meaning
────────────────    ──────────────────────────────────────────────────────
MUST / REQUIRED     Absolute requirement. No exceptions.
SHALL               Implementations MUST do this or they are non-compliant.

MUST NOT /          Absolute prohibition.
SHALL NOT           Doing this makes the implementation non-compliant.

SHOULD /            Strong recommendation.
RECOMMENDED         Valid reasons may exist to ignore this in specific
                    circumstances, but full implications must be understood
                    and carefully weighed before choosing differently.

SHOULD NOT /        Strong recommendation against.
NOT RECOMMENDED     Same weight as SHOULD but in the negative direction.

MAY / OPTIONAL      Truly optional. Implementation may or may not do this.
                    Interoperability must be maintained either way.
```

##### Why this matters in practice

```
RFC 793 (TCP): "A TCP MUST be able to receive a TCP option"
→ Your TCP implementation has no choice. Must support options.

RFC 5681 (Congestion Control): "TCP SHOULD use slow start"
→ Strong recommendation. You could deviate with good reason,
  but you had better understand why.

RFC 793: "A TCP MAY implement urgent data"
→ Optional feature. Fine to skip.
```

When reading an RFC, the capitalization of these words is intentional and load-bearing. `must` (lowercase) is just English. `MUST` (uppercase) is a protocol requirement.

---

#### RFCs relevant to what you have studied in CS144

```
RFC Number   Topic                              Relevance to CS144
──────────   ─────────────────────────────────  ───────────────────────────
RFC 791      Internet Protocol (IPv4)           IP layer, packet format
RFC 793      Transmission Control Protocol      TCP fundamentals, your labs
RFC 826      ARP                                NetworkInterface lab
RFC 1122     Requirements for Internet Hosts    TCP behavior rules
RFC 2119     Key words (MUST/SHOULD/MAY)        Reading any RFC
RFC 5681     TCP Congestion Control             Unit 4, Tahoe/Reno/NewReno
RFC 6298     TCP Retransmission Timer           RTT estimation, RTO
RFC 6582     TCP New Reno                       New Reno partial ACK
RFC 1058     RIP                                Distance vector routing
RFC 4271     BGP-4                              BGP, interdomain routing
```

---

#### How to read an RFC

RFCs have a standard structure:

```
┌──────────────────────────────────────────────────────┐
│  RFC XXXX                            Month Year       │
│  Title                                               │
│  Author(s)                                           │
│  Status: Standards Track / Informational / etc.      │
├──────────────────────────────────────────────────────┤
│  Abstract        ← 1 paragraph summary               │
├──────────────────────────────────────────────────────┤
│  1. Introduction ← motivation and scope              │
│  2. ...          ← protocol definitions              │
│  3. ...          ← detailed behavior                 │
│  ...                                                 │
│  N. Security Considerations  ← required section      │
│  N+1. IANA Considerations    ← registry assignments  │
│  References                  ← normative + informative│
└──────────────────────────────────────────────────────┘
```

##### Practical reading tips

```
1. Read the Abstract first.
   Tells you what the RFC does in one paragraph.

2. Check the Status.
   "Proposed Standard" vs "Internet Standard" vs "Informational"
   tells you how seriously to treat it.

3. Look for the terminology section.
   Defines key terms used throughout the document.

4. Pay attention to MUST/SHOULD/MAY.
   These are the actual protocol requirements.

5. Check the date and Obsoletes/Updates fields.
   RFCs are never edited after publication.
   A newer RFC may supersede parts of an older one.
   Example: RFC 6298 obsoletes RFC 2988 for RTO computation.

6. Use the index at tools.ietf.org.
   Search by RFC number, keyword, or working group.
```

---

#### An important property: RFCs never change

Once an RFC is published, it is **permanent and immutable**. It can never be edited. If a correction is needed:

```
Option 1: Errata
  Small factual errors are noted as "errata" on the RFC's page.
  The RFC text itself is unchanged.

Option 2: New RFC that Updates the old one
  The new RFC says "Updates: RFC XXXX"
  The old RFC says "Updated by: RFC YYYY"
  Both remain published.

Option 3: New RFC that Obsoletes the old one
  The new RFC says "Obsoletes: RFC XXXX"
  The old RFC is marked "Obsoleted by: RFC YYYY"
  The old RFC is still readable but no longer current.
```

This is why TCP has many RFCs spread across decades:

```
RFC 793  (1981) → original TCP
RFC 1122 (1989) → corrections to host requirements
RFC 5681 (2009) → congestion control (obsoletes RFC 2581)
RFC 6298 (2011) → RTO computation (obsoletes RFC 2988)
RFC 6582 (2012) → New Reno (obsoletes RFC 3782)
```

All of these together define modern TCP behavior. No single RFC tells the complete story.

---

#### Summary

```
┌────────────────────────────────────────────────────────────────┐
│                    What an RFC is                              │
│                                                                │
│  RFC = Request for Comments                                    │
│  Published by: IETF (Internet Engineering Task Force)          │
│  Purpose: define exactly how Internet protocols work           │
│                                                                │
│  Types: Standards Track, Informational,                        │
│         Experimental, BCP, Historic                            │
│                                                                │
│  Key terminology (RFC 2119):                                   │
│    MUST     = absolute requirement                             │
│    SHOULD   = strong recommendation                            │
│    MAY      = optional                                         │
│                                                                │
│  Properties:                                                   │
│    Never edited after publication                              │
│    Superseded by newer RFCs (Updates / Obsoletes)             │
│    Publicly available at tools.ietf.org                        │
│    Over 9,000 published since 1969                             │
│                                                                │
│  Relation to CS144:                                            │
│    Everything you implement in the labs has an RFC behind it   │
│    RFC 793 = TCP you are building                              │
│    RFC 5681 = congestion control you studied in Unit 4         │
│    RFC 826  = ARP in your NetworkInterface lab                 │
└────────────────────────────────────────────────────────────────┘
```

In short: RFCs are the constitution of the Internet. If you want to know the ground truth about how any Internet protocol works — not a textbook's simplification, but the actual specification — you read the RFC.



## 17. Summary

### The complete picture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        TCP Congestion Control                       │
│                                                                     │
│  Problem: Don't collapse the network; share fairly                  │
│  Solution: AIMD — probe for bandwidth, back off on loss             │
│                                                                     │
│  TCP Tahoe (1988):                                                  │
│    ✓ Congestion window (cwnd)                                       │
│    ✓ Slow start (exponential probe)                                 │
│    ✓ Congestion avoidance (linear AIMD)                             │
│    ✓ ssthresh (boundary between the two)                            │
│    ✓ RTT estimation with variance (EWMA + variance)                 │
│    ✓ Self-clocking (ACK-driven pacing)                              │
│    ✓ Fast retransmit (3 dup ACKs → immediate retransmit)            │
│                                                                     │
│  TCP Reno (1990):                                                   │
│    ✓ Fast recovery (halve cwnd, not reset to 1)                     │
│    ✓ Congestion window inflation (keep pipe full during recovery)   │
│                                                                     │
│  Key equations:                                                     │
│    Throughput   R = √(3/2) / (RTT × √p)                            │
│    Buffer size  B = RTT × C                                         │
│    Window       = min(cwnd, flow control window)                    │
└─────────────────────────────────────────────────────────────────────┘
```

### Key insights to internalize

1. **Congestion is unavoidable** in a shared network — the goal is to manage it, not eliminate it.
2. **AIMD converges to fairness** through the geometry of additive increase and multiplicative decrease.
3. **RTT penalization** is a fundamental limitation of AIMD — long-RTT flows get less bandwidth.
4. **Self-clocking** is elegant: ACKs naturally pace the sender to match the bottleneck rate.
5. **ssthresh** is TCP's memory of where congestion last occurred.
6. **Fast recovery** keeps TCP in congestion avoidance, maintaining true AIMD behavior in steady state.
7. **Buffer sizing** (RTT × C) is critical for performance — too small and the link goes idle; too large and latency explodes.

### TCP versions at a glance

| Feature | Pre-Tahoe | Tahoe | Reno |
|---|---|---|---|
| Congestion window | No | Yes | Yes |
| Slow start | No | Yes | Yes |
| Congestion avoidance | No | Yes | Yes |
| Good RTT estimation | No | Yes | Yes |
| Self-clocking | No | Yes | Yes |
| Fast retransmit | No | Yes | Yes |
| Fast recovery | No | No | Yes |
| Window inflation | No | No | Yes |
| Steady-state AIMD | No | Approx. | Yes |

---

*Based on Stanford CS144 Unit 4 lecture materials by Nick McKeown and Phil Levis.*
