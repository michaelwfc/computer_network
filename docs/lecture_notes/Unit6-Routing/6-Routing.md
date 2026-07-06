# The Routing Basics 

## Routing Problem:
How should packets from A reach B in the network?

## Four Basic Routing Approaches 
- Flooding 
- Source routing 
- Forwarding table 
- Spanning tree 











### 1. Flooding




Flooding is the simplest possible routing algorithm.

Instead of trying to determine the correct path, every router simply forwards the packet to **every outgoing link except the one on which it arrived**.

```
  Packet
  |
Router
/  |  \
  /|\
 Link1 Link2Link3
```

Every neighboring router repeats the same process.

Eventually, one copy reaches the destination.

#### Why Flooding Works


Flooding requires almost no knowledge of the network.

Routers do not need routing tables.

They do not even need to know where the destination is.

As long as the network remains connected, at least one copy of the packet will eventually arrive.

This makes flooding extremely robust.

---

#### Problems with Flooding
• Inefficient link usage. 
• Packets can loop forever. 
• Used when we don’t know (or can’t trust) the topology. 


Unfortunately, flooding creates several serious problems.

##### Massive Duplicate Traffic

Every router generates multiple copies of the packet.

Those copies generate even more copies.

Traffic grows exponentially.

For example:

```
1 packet
↓
3 copies
↓
9 copies
↓
27 copies
↓
81 copies
```

A single packet can quickly consume enormous amounts of bandwidth.

---

##### Packets Can Loop Forever

Suppose the network contains a cycle.

```
A ---- B
|      |
|      |
D ---- C
```

Without additional rules, packets can circulate forever.

Real flooding systems therefore add mechanisms like:

* Time-To-Live (TTL)
* Sequence numbers
* Duplicate detection

to eventually discard old packets.

---

#### When Is Flooding Useful?

Although inefficient, flooding is still valuable in certain situations.

For example:

* discovering unknown network topology
* broadcasting information
* emergency routing after failures
* military or ad hoc wireless networks where topology changes rapidly

Whenever routers cannot trust their routing information, flooding provides a reliable fallback.

---

> **Key Takeaway:** Flooding is simple and highly reliable because it requires almost no network knowledge, but it is extremely inefficient due to duplicated packets and potential routing loops.

---


### 2. Source Routing
• “End-­‐to-­‐end” solution – no support needed from network. 
• Packet carries a variable (and maybe long) list of addresses. 
• End host must know the topology and choose the route. 
• Used when end user wants to control the route.

Instead of letting routers decide the path, **the sender decides the entire route before sending the packet**.

The packet itself carries the complete list of routers that should be visited.

For example,

```
Packet:

Destination:
B

Route:
R1 → R4 → R7 → R9
```

Each router simply reads the next address from the packet and forwards it accordingly.

---

#### Advantages

The network becomes much simpler.

Routers no longer need to compute routes.

The sender has complete control over the path.

This allows users to:

* avoid congested links
* test network paths
* choose trusted routers
* implement traffic engineering

---

#### Disadvantages

The packet header becomes larger because every hop must be recorded.

Even more importantly, the sender must already know the network topology.

In today's Internet, this is unrealistic because:

* networks constantly change
* links fail
* congestion appears dynamically
* routers join and leave

Maintaining an up-to-date global map of the Internet is practically impossible for end hosts.

---

#### Where Source Routing Is Used

Although rare in today's public Internet, source routing still appears in specialized systems:

* Software-defined networking (SDN)
* Segment Routing (SR)
* Data center networks
* Network debugging tools

---

> **Key Takeaway:** Source routing shifts routing decisions from the network to the sender. It offers maximum control but requires the sender to know the network topology in advance.

---
### 3. Forwarding Tables

• An optimization: Network handles hop-­‐by-­‐hop routing. 
• Requires population of forwarding tables. 
• Per-­‐destination state, not (necessarily) per-­‐flow state. 
Modern IP networks almost always use **forwarding tables**.

Instead of storing an entire route inside every packet, routers maintain local tables that answer one simple question:

> "If the destination is X, which neighbor should receive the packet?"

Example forwarding table:

| Destination | Next Hop |
| ----------- | -------- |
| Network A| Router 2 |
| Network B| Router 5 |
| Network C| Router 3 |

When a packet arrives, the router performs a table lookup.

```
Packet arrives
 ↓
Lookup destination
 ↓
Find next hop
 ↓
Forward packet
```

Each router only decides the **next hop**, not the complete path.

This is known as **hop-by-hop routing**.

---

#### Why Is This Better?

Each packet remains small.

The sender does not need to understand the network.

Routers can react quickly to failures by updating their forwarding tables.

This makes forwarding tables highly scalable.

---

#### Forwarding Tables vs. Per-Flow State

Notice that routers typically maintain information **per destination**, not **per connection**.

For example:

```
Destination Network 192.168.10.0/24

↓

Always forward to Router 5
```

Whether one computer or one million computers are communicating with that destination, the same forwarding entry can be reused.

This greatly reduces memory usage.

---

> **Key Takeaway:** Modern routers perform hop-by-hop forwarding using forwarding tables. This scales far better than storing complete routes inside every packet.

### 4. Spanning Trees

- Spanning: It reaches all leaves 
- Tree: It has no loops 


Many networks contain loops.

```
A ----- B
|       |
|       |
D ----- C
```

Loops can cause packets to circulate forever.

A **tree** is a connected graph without cycles.

A **spanning tree** is a tree that includes every router in the network.

```
Original graph

A ----- B
|       |
|       |
D ----- C


Spanning tree

A ----- B
|
|
D ----- C
```

One link has been removed.

Now there are no loops.

Every node remains reachable.

---

#### Why Are Spanning Trees Important?

Many routing algorithms first compute a spanning tree before forwarding packets.

Benefits include:

* no forwarding loops
* guaranteed connectivity
* simpler routing
* easier reasoning about paths

---

> **Key Takeaway:** A spanning tree connects every node while eliminating cycles, providing a loop-free foundation for routing.

---


## How Do We Choose the "Best" Route?

There are often multiple possible paths between two computers.

For example:

```
A → R1 → R2 → B

or

A → R3 → R4 → R5 → B
```

Which one should be chosen?

The answer depends on the routing **metric**.


### Metrics

Choices 
- Min distance 
- Min hop-­‐count 
- Min delay 
- Max throughput 
- Least-‐loaded path 
- Most reliable path 
- Lowest cost path 
- Most secure path 
- ... 



A metric is a numerical value that measures how "good" a path is.

Common routing metrics include:

| Metric | Goal  |
| ------------------ | -------------------------- |
| Minimum distance| Shortest physical path  |
| Minimum hop count  | Fewest routers crossed  |
| Minimum delay| Fastest delivery  |
| Maximum throughput | Highest bandwidth |
| Least-loaded path  | Avoid congestion  |
| Most reliable path | Lowest failure probability |
| Lowest cost  | Cheapest transmission|
| Most secure path| Highest security  |

Different routing protocols optimize for different metrics.

---



## Shortest-Path Spanning Trees

The most common routing objective is to find the **minimum-cost path** from every router to every destination.

Rather than minimizing physical distance alone, "cost" can represent many factors, including delay, bandwidth, congestion, or administrative preferences.

The result is a **shortest-path spanning tree**, where each router selects the next hop that minimizes the total accumulated cost to the destination.

Modern routing protocols such as **OSPF** and **IS-IS** build shortest-path trees using graph algorithms like **Dijkstra's algorithm**, allowing routers to compute efficient, loop-free routes.

---

> **Key Takeaway:** Modern routing protocols typically compute shortest-path spanning trees, where "shortest" means the path with the lowest total routing cost rather than simply the fewest kilometers.

---

## Beyond Basic Routing

Not every application benefits from sending all traffic along a single path.

Two important extensions are widely used in modern networks.

### Multipath Routing

Instead of selecting only one path, routers may use several equally good paths simultaneously.

Benefits include:

* better bandwidth utilization
* improved fault tolerance
* automatic load balancing

This technique is commonly known as **Equal-Cost Multi-Path (ECMP)** routing.

---

### Multicast Routing

Sometimes the sender wants to deliver identical data to many receivers simultaneously.

Examples include:

* live video streaming
* online gaming
* financial market feeds
* video conferencing

Instead of sending separate copies to every receiver, multicast duplicates packets only when paths diverge, greatly reducing bandwidth consumption.

---

> **Key Takeaway:** Multipath routing improves performance by distributing traffic across multiple routes, while multicast routing efficiently delivers the same data to many receivers.

---

## Summary

Routing is the process of determining how packets travel from a source to a destination through a network of routers. Although the fundamental question is simple, designing scalable, efficient, and reliable routing algorithms is one of the central challenges of computer networking.

The key ideas from this chapter are:

1. **Flooding** forwards packets on every link and guarantees delivery if the network is connected, but wastes bandwidth and can create loops.
2. **Source routing** allows the sender to specify the entire path, giving complete control at the cost of larger packet headers and requiring global topology knowledge.
3. **Forwarding tables** are the foundation of modern IP routing, enabling routers to make efficient hop-by-hop forwarding decisions.
4. **Spanning trees** eliminate loops while maintaining connectivity, providing a clean structure for routing algorithms.
5. Modern routing protocols compute **shortest-path spanning trees** using routing metrics such as delay, bandwidth, reliability, or cost, and advanced techniques like **multipath routing** and **multicast routing** further improve network performance for specialized applications.

Understanding these basic routing strategies provides the conceptual foundation for studying routing protocols such as RIP, OSPF, IS-IS, and BGP, which determine how the Internet routes billions of packets every second.



# Distance Vector Protocol: Bellman Ford algorithm 

## Problem: How can routers work together  
to find minimum cost spanning tree?  

Equivalent to finding minimum  
cost spanning tree among routers only 

Questions: 
1. What is the maximum run time of the 
algorithm? 
2. Will the algorithm always converge? 
3. What happens when link costs change, or when 
routerstilinks fail?


Counting to Infinity Problem Solutions 
1. Set infinity = “some small integer” (e.g. 16).  
Stop when count = 16. 
2. Split Horizon: Because R2 received lowest cost path 
from R3, it does not advertise cost to R3. 
3. Split-­‐horizon with poison reverse: R2 advertises 
infinity to R3. 
4. There are many problems with (and fixes for) the 
Bellman-­‐Ford algorithm


## Bellman Ford in practice 
Bellman-­‐Ford algorithm is an example of a Distance 
Vector algorithm. 
It was used in the first Internet routing protocol, 
called Routing Information Protocol (RIP). 
It requires very ligle computation on the routers, is 
distributed, and will eventually converged. 
Over time it was replaced by algorithms that 
calculate the entire spanning tree at each router.




# Internet Routing: 
Autonomous Systems, BGP, Multicast, Spanning Trees, and IPv6

## Introduction

In the previous chapter, we learned the basic problem of routing: **how does a packet travel from one computer to another across a network?** We explored several routing strategies such as flooding, source routing, forwarding tables, and shortest-path trees.

However, the modern Internet is far larger than a single network. It consists of **tens of thousands of independently operated networks**, each owned by Internet Service Providers (ISPs), universities, cloud providers, governments, and enterprises. These organizations all have different goals, different internal network designs, and different business relationships.

This raises a much bigger question:

> **How can thousands of independent networks cooperate to deliver packets across the entire Internet?**

The answer is a hierarchical routing architecture built around **Autonomous Systems (ASes)** and specialized routing protocols. Inside an AS, routers cooperate using **Interior Gateway Protocols (IGPs)** such as RIP and OSPF. Between ASes, routers exchange reachability information using the **Border Gateway Protocol (BGP)**, the protocol that effectively "holds the Internet together."

This chapter also introduces multicast routing, Ethernet's Spanning Tree Protocol (STP), and IPv6, the successor to IPv4.

---

The Internet Is Too Large for One Routing Algorithm

Imagine trying to maintain a single routing table containing every router on Earth.

Today, the Internet contains:

* Millions of networks
* Hundreds of millions of routers
* Constant topology changes
* Organizations with conflicting business interests

A single global routing algorithm would not scale.

Instead, the Internet is divided into many smaller administrative domains called **Autonomous Systems**.

---

## Autonomous Systems (AS)




An **Autonomous System (AS)** is a collection of IP networks and routers managed by a **single administrative organization** that presents a common routing policy to the Internet.

Examples include:

| Organization        | Example AS Number |
| ------------------- | ----------------: |
| Stanford University |                32 |
| Google              |             15169 |
| AT&T                |               797 |
| Cloudflare          |             13335 |
| Amazon              |             16509 |

Each AS receives a globally unique **Autonomous System Number (ASN)**.

Think of an AS as a country.

* Inside the country, the government decides how roads are built.
* Between countries, everyone follows international rules.

Similarly,

* **Inside an AS**, the owner chooses any routing protocol.
* **Between ASes**, everyone must speak **BGP-4**.


### Characteristics of AS 

The basic unit of hierarchy in the Internet. 
- Within an AS, the owner decides how routing is done 
- Between AS’s, must use BGP-­‐4 (Border Gateway Protocol, v4)* 


---



### Finding an Autonomous System

Several networking tools allow you to identify an AS.

Finding an AS number 
- Query DNS to find IP address at Stanford: `dig stanford.edu` 
Returns: “stanford.edu  1800 IN A 171.64.13.26” 
- Find AS for IP address: `nc whois.cymru.com 43`  
Enter: 171.64.13.26
  Stanford (32), AT&T (797), Google (15169, 22859, 36039)  
- `traceroute –a <destination>` will report AS numbers.

#### DNS Lookup

First obtain the IP address.

```bash
dig stanford.edu
```

Example output:

```text
stanford.edu. 1800 IN A 171.64.13.26
```


```bash

cs144@cs144vm:~/computer_network$ dig stanford.edu

; <<>> DiG 9.20.11-0ubuntu0.2-Ubuntu <<>> stanford.edu
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 24115
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 65494
;; QUESTION SECTION:
;stanford.edu.                  IN      A

;; ANSWER SECTION:
stanford.edu.           1800    IN      A       171.67.215.200

;; Query time: 1238 msec
;; SERVER: 127.0.0.53#53(127.0.0.53) (UDP)
;; WHEN: Sat Jul 04 07:47:57 UTC 2026
;; MSG SIZE  rcvd: 57

```
---

#### WHOIS Lookup

Determine which AS owns that address.

```bash
nc whois.cymru.com 43
```

Then enter:

```text
171.64.13.26
```
The server returns the ASN responsible for that address.

```bash
cs144@cs144vm:~/computer_network$ nc whois.cymru.com 43
171.67.215.200
AS      | IP               | AS Name
32      | 171.67.215.200   | STANFORD - Stanford University, US
```

---

#### Traceroute

Modern traceroute implementations can also display AS numbers.

```bash
traceroute -a example.com
```

This allows you to observe how packets travel through multiple autonomous systems across the Internet.

```bash


```
---

> **Key Takeaway:** The Internet is divided into Autonomous Systems. Each AS manages its own internal routing while communicating with other ASes using BGP.

---

## Interior Routing Protocols (IGPs)


Routing **inside** an Autonomous System is called **interior routing**.

Because every router belongs to the same organization, routers generally trust each other and can cooperate closely.

Two major Interior Gateway Protocols are RIP and OSPF.

---

### RIP (Routing Information Protocol)
 
RIP is one of the oldest routing protocols on the Internet.

It uses the **Distance Vector algorithm**, which is based on the distributed **Bellman–Ford shortest-path algorithm**.

Every router periodically tells its neighbors:

> "Here is my current estimate of the distance to every destination."

Neighbors update their own routing tables based on this information.

---

#### Characteristics of RIP

* Uses distance vector (distributed Bellman-­‐Ford algorithm).
* RFC 2453
* Updates every 30 seconds
* Very simple implementation
* Originally shipped with BSD Unix (`routed`)
* No authentication for updates in the original protocol
* Widely used for many years; used less now.

Because updates occur on a fixed timer, convergence after failures can be relatively slow.

---

#### Advantages

* Easy to configure
* Low CPU requirements
* Suitable for small networks

---

#### Limitations

* Slow convergence
* Limited scalability
* Maximum hop count of 15
* Periodic broadcasts consume bandwidth
* Originally lacked security

Consequently, RIP has largely been replaced by more sophisticated protocols.

---

> **Key Takeaway:** RIP is a simple distance-vector protocol that was historically important but is now mostly used in small or legacy networks.

---

### OSPF (Open Shortest Path First)



OSPF is the most widely deployed Interior Gateway Protocol today.

Instead of exchanging distances, routers exchange **Link State Advertisements (LSAs)** describing the state of their directly connected links.

These advertisements are flooded throughout the Autonomous System.

As a result, every router eventually possesses the same complete map of the network.

Each router independently computes the shortest paths using **Dijkstra's algorithm**.

---

#### Characteristics

* Link-­‐state updates sent (using flooding) as and when required. 
* RFC 2328
* Every router runs Dijkstra’s algorithm. 
* Event-driven updates
* Authenticated updates. Authenticated routing messages
* Hierarchical design using **Areas**, Autonomous system may be partitioned into “areas”. 
* Fast convergence
* Widely used, complex. 
* IS-­‐IS (RFC 1142) is similar, and is also widely used. 
Because every router computes routes independently from the same topology database, routing decisions remain consistent throughout the network.

---

#### OSPF vs RIP

| Feature        | RIP              | OSPF                      |
| -------------- | ---------------- | ------------------------- |
| Algorithm      | Bellman-Ford     | Dijkstra                  |
| Routing Type   | Distance Vector  | Link State                |
| Updates        | Every 30 seconds | When topology changes     |
| Scalability    | Small networks   | Large enterprise networks |
| Authentication | Originally none  | Supported                 |
| Convergence    | Slow             | Fast                      |

Another widely used link-state protocol, **IS-IS**, operates similarly and is especially common in ISP backbones.

---

> **Key Takeaway:** OSPF builds a complete map of the network and computes shortest paths locally, making it much faster and more scalable than RIP.

---

## Routing Inside an AS

Not every Autonomous System connects to the Internet in the same way.

The number of external connections determines how internal routing is performed.

---

### Single Exit Point

Many organizations connect to the Internet through a single gateway.

```text
          Internet
              |
        Border Router
              |
      ----------------
      |              |
    Router A      Router B
```

Since there is only one way out, routing is simple.

Every router knows:

* all internal destinations
* one default route toward the border router

Any packet destined for another AS is simply forwarded to that default gateway.

This keeps routing tables small and configuration simple.

#### Charactertics of Routing to a single exit point 
There is only one exit point, so routers within the AS can use default routing.  
- Each router knows all prefixes within AS. 
- Packets for another AS are sent to the default router.  
- Default router is the border gateway to the next AS. 
Routing tables in single exit AS’s tend to be small. 
---

### Multiple Exit Points

Large enterprises and Internet Service Providers usually have several Internet connections.

```text
            ISP A
              |
          Border 1

Enterprise Network

          Border 2
              |
            ISP B
```

Now routers must decide:

> Which exit should this packet use?

Two common strategies exist.


#### Charactertics of Routing to multiple exit points 
Used by multi-­‐homed enterprises and transit AS’s. 
Each internal router must be told which exit point to 
use for a given destination prefix. 
Requires large routing tables to route to every prefix. 
Approach 1: Hot-­‐potato routing – send to closest exit. 
Approach 2: Pick exit closest to destination.


#### Hot Potato Routing

Forward the packet to the **nearest exit**.

The goal is to move traffic out of your network as quickly as possible.

This minimizes your own resource usage.

---

#### Cold Potato Routing

Keep the packet inside your network until it reaches the exit closest to the destination.

This gives the organization greater control over routing quality but uses more internal bandwidth.

---

> **Key Takeaway:** Networks with multiple Internet connections must choose not only the path to a destination but also which border router should carry the traffic out of the AS.

---

## Exterior Routing 

### Internal vs External Routing

This separation greatly simplifies Internet routing.

```text
               Internet

        +--------------------+
        |                    |
      AS 100             AS 200
   (University)         (Google)
        |                    |
        |                    |
      AS 300             AS 400
         ISP             Enterprise
```

Each AS hides its internal complexity from the rest of the Internet.

Other ASes only need to know:

> "How can I reach your network?"

They do **not** need to know every internal router.


---


### Charactertics of Exterior Routing Protocol 

Every AS must interconnect using BGP-­‐4. 
Problems BGP-­‐4 designed to solve 
- Topology: The Internet is a complex mesh of different AS’s with very little structure. 
- Autonomy of AS’s: Each AS defines link costs in different ways, so 
not possible to find lowest cost paths. 
- Trust: Some AS’s can’t trust others to advertise good routes (e.g. 
two competing backbone providers), or to protect the privacy of 
their traffic (e.g. two warring nations). 
- Policies: Different AS’s have different objectives (e.g. route over 
fewest hops; use one provider rather than another).

## The Structure of the Internet  

``` 
AT&T                  NTT
(Tier 1 ISP )         (Tier 1 ISP )

Regional ISP          Regional ISP

Access ISP            Access ISP


User
```

Your diagram captures the right shape, but let me build it up from first principles so you understand *why* it's shaped this way, not just *that* it is.

### 1. Intuition: Why hierarchy at all?

The Internet is not one network — it's a **network of independently-operated networks** (Autonomous Systems, or ASes), each with its own routers, links, and business interests. Nobody centrally designed the topology. It emerged from an economic problem:

> Every network wants to reach every other network, but it's economically infeasible for every network to have a direct physical link to every other network.

If there are N networks and each needs a direct connection to every other, you need O(N²) links. That doesn't scale. So instead, smaller networks pay larger networks to carry their traffic to the rest of the Internet — this is **transit**. And large networks that see mutual benefit connect directly to each other for free — this is **peering**. Out of these two relationships, a rough hierarchy emerges.

### 2. The mechanism: Tiers and relationships

```text
                    ┌─────────────┐         ┌─────────────┐
                    │   Tier 1    │◄───────►│   Tier 1    │
                    │  (AT&T)     │  peering│   (NTT)     │
                    └──────┬──────┘         └──────┬──────┘
                           │ transit               │ transit
                           ▼                        ▼
                    ┌─────────────┐         ┌─────────────┐
                    │  Regional   │◄───────►│  Regional   │
                    │    ISP      │ peering │    ISP      │
                    └──────┬──────┘         └──────┬──────┘
                           │ transit               │ transit
                           ▼                        ▼
                    ┌─────────────┐         ┌─────────────┐
                    │   Access    │         │   Access    │
                    │    ISP      │         │    ISP      │
                    └──────┬──────┘         └──────┬──────┘
                           │                        │
                           ▼                        ▼
                        User                      User
```

**Tier 1 ISPs** (AT&T, NTT, Lumen, Telia, Zayo, etc.)
- Definition: an AS that can reach **every other network on the Internet using only settlement-free peering** — it never has to *pay* anyone for transit.
- They form the "core" — a dense mesh of peering links between a few dozen massive networks.
- This is a *commercial* definition, not a technical one. There's no protocol field that says "I am Tier 1." It's an emergent property of who peers with whom.

**Regional ISPs (Tier 2)**
- Buy transit from one or more Tier 1s to reach the global Internet.
- Often also peer with other Tier 2s directly (especially ones carrying a lot of mutual traffic), which reduces cost and latency for that traffic without going through Tier 1s.
- Provide service to Access ISPs or to end customers over a wide geography.

**Access ISPs (Tier 3)**
- The network you and I actually connect to (Comcast last-mile, a mobile carrier, a campus network).
- Almost always **pure customers** — they pay for transit and rarely peer with anyone, since they don't have enough symmetric traffic to make peering worthwhile for the upstream.

**User**
- Not an AS at all (usually). Connects via the access ISP's link — DSL, cable, fiber, cellular.

### 3. Implementation detail: how "reachability" actually happens — BGP

This hierarchy isn't enforced by any central authority; it's the *result* of how the **Border Gateway Protocol (BGP)** works.

- Each AS has a number (ASN) and advertises which IP prefixes it can reach.
- A customer AS advertises its prefixes to its provider, which then re-advertises them further up.
- A provider AS advertises *its own* prefixes and its *customers'* prefixes to its peers — but critically, **it does not advertise one peer's routes to another peer**. This is the "no valley" or "Gao-Rexford" routing policy:

```text
Valid path:    customer → provider → provider → customer   (transit, paid)
Valid path:    customer → provider → peer → customer        (peering in the middle, free)
INVALID:       peer → provider → peer   (a peer would never carry your traffic for free
                                          to reach another peer — no economic incentive)
```

This policy is *why* the hierarchy is self-reinforcing: a network only carries traffic for free (via peering) between its own peers/customers, never as a value-add fare between two other people it's connected to. Traffic between two unrelated Tier 2s, absent direct peering, must transit up through a common Tier 1.

### 4. Where your diagram is a simplification

A few important nuances CS144 (and real operational practice) will push you toward:

**(a) It's not strictly a tree — it's a graph with cycles.**
Real ASes multi-home: an access ISP might buy transit from *two* different regional ISPs for redundancy. Regional ISPs peer with each other directly. So paths between two hosts are chosen dynamically by BGP policy and route availability, not a fixed hierarchy.

**(b) Internet Exchange Points (IXPs)**
Rather than every peering relationship being a dedicated physical link, many networks connect once to a shared physical switching fabric (an IXP — e.g., DE-CIX, LINX, Equinix) and peer with dozens of other networks over that single connection. This flattens a lot of the "large ISP-to-large ISP" peering into a hub-and-spoke at the IXP level.

**(c) CDNs have partially flattened the hierarchy**
Content providers like Google, Netflix, Cloudflare, and Akamai now run their own global backbone networks and place servers (or entire mini-datacenters, e.g. Netflix Open Connect) *directly inside* access ISPs. This means a huge fraction of user traffic (video, in particular) never traverses a Tier 1 at all — it goes:

```text
User → Access ISP → (Netflix cache co-located inside the Access ISP's own network)
```

This matters practically: it's a major reason why "the Internet" today has much shorter, cheaper, lower-latency paths for high-volume content than the classic three-tier diagram would suggest.

**(d) "Tier" is a rough business classification, not a protocol layer**
Don't confuse this hierarchy (an AS-level, economic/topological structure) with the **OSI/TCP-IP protocol layers** (link, network, transport, application) that CS144 spends most of its time on. Both are called "layers"/"tiers" colloquially, but they answer completely different questions:

| | Tier 1/2/3 hierarchy | Protocol layers (IP, TCP, etc.) |
|---|---|---|
| Question answered | *Who* carries my packet, and who pays whom? | *What information* is attached to my packet at each stage? |
| Unit of analysis | Autonomous Systems (business entities) | Header/payload structure on a single link or end-to-end path |
| Enforced by | BGP policy + business contracts | Protocol specifications (RFCs) |

### Summary intuition to keep

The Internet's structure is an **economic equilibrium**, not an engineering blueprint. Every AS is trying to answer: "Who do I need to pay to reach the rest of the world, and who is it worth connecting to for free?" Tier 1/2/3 is the label we give to the answer that equilibrium settles into. The actual bits-on-the-wire mechanism that makes this reachability *work* — how routes propagate, how a router picks a next hop, how failures are handled — is BGP, which is worth studying next if you want to go from "structural picture" to "mechanism."

-----

# Border Gateway Protocol (BGP-­‐4)

BGP is the one protocol that makes "the Internet" a single reachable graph out of tens of thousands of independently-operated networks. It is not a routing protocol in the OSPF/RIP sense — it's a distributed policy negotiation carried out over TCP. 

BGP is the routing protocol that connects Autonomous Systems together.Unlike RIP or OSPF, BGP does **not** attempt to compute shortest paths based on link costs.
Instead, BGP is primarily concerned with **reachability and routing policy**.

## Why BGP Exists
Inside a single organization's network — a campus, a datacenter, an ISP's own backbone — you can run an Interior Gateway Protocol (OSPF, IS-IS) because everyone trusts everyone. Every router is owned by the same entity, every metric (link cost, bandwidth) is comparable, and the goal is simple: find the shortest or cheapest path to every destination inside the network.

The Internet is the opposite situation. It's roughly 70,000+ independently-operated networks, each called an Autonomous System (AS), each with its own equipment, its own economics, and — critically — its own opinion about which routes it is willing to use and which it is willing to advertise to others. AT&T does not trust NTT's internal link-cost metrics, does not want NTT dictating its routing policy, and has business reasons (money) to prefer some paths over others that have nothing to do with hop count or latency.

Core intuition
An IGP answers "what is the shortest path?" 
BGP answers "what is the path I am willing to use, given who I trust and who is paying whom?" Distance is almost incidental to BGP's decision process — policy dominates.

This is why BGP is called a **path-vector protocol** rather than a distance-vector or link-state protocol. It doesn't compute shortest paths from a cost metric like RIP or OSPF. Instead, every AS advertises to its neighbors: "I can reach this prefix, and here is the exact sequence of ASes the traffic will pass through to get there." Each recipient AS then applies its own local policy to decide whether to use that path, prefer it over alternatives, and whether to re-advertise it onward.


### Charactertics of BGP

BGP is not a link-­‐state or distance-­‐vector routing protocol. 
- Instead, BGP uses what is called a “Path vector” 
BGP routers advertise complete paths (a list of AS’s). 
- Also called AS_PATH (this is the path vector) 
- Example of path advertisement: 
“The network 171.64ti16 can be reached via the path {AS1, AS5, AS13}” 
Paths with loops are detected locally and ignored. 
Local policies pick the preferred path among options. 
When a linktirouter fails, the path is “withdrawn”. 



---

### Why Doesn't BGP Use Shortest Paths?

Different organizations have different objectives.

For example:

* An ISP may prefer cheaper transit providers.
* A company may avoid sending traffic through competitors.
* Governments may prohibit certain international routes.
* Military networks may require trusted paths.

Therefore, there is no universally agreed definition of "best path."

BGP allows each AS to make its own policy decisions.

### Why BGP Is Policy-Based

When multiple paths exist,

```text
Path A:
100 → 200 → 300

Path B:
500 → 600
```

the shorter AS path is **not always selected**.

Local routing policies may instead prefer:

* lower cost providers
* trusted partners
* customer routes
* higher bandwidth
* contractual agreements

Policy often matters more than distance.

---

> **Key Takeaway:** BGP is a path-vector routing protocol that exchanges complete AS paths and allows every Autonomous System to enforce its own routing policies.


---

## Path Vector Routing

BGP introduces a different routing model called the **Path Vector** algorithm.

Instead of advertising only a distance, routers advertise the **entire sequence of Autonomous Systems** leading to a destination.

### AS numbers as the unit of trust
Every AS has a globally unique ASN (Autonomous System Number, a 32-bit value since RFC 6793 extended the original 16-bit space). ASN 7018 is AT&T, ASN 2914 is NTT. BGP doesn't reason about individual routers — it reasons about ASes as atomic units. Everything inside AS 7018 is AT&T's problem to route internally; BGP only cares about the boundary.

### The AS_PATH attribute
Every route advertisement carries an `AS_PATH` — an ordered list of the ASes the announcement has passed through, most-recently-added AS first. When an AS advertises a prefix to an external neighbor, it prepends its own ASN to the AS_PATH before sending.


Example:

```text
Destination:  171.64.0.0/16
Prefix Originated

AS_PATH:
AS 6341 -> AS 7018 -> AS 1239 -> AS 1755 -> AS 1129 -> AS 12654
64500 → 3356 → 32
```

This means the destination can be reached through AS_PATH and finally Stanford's AS 32.



This single field solves two problems at once, which is worth sitting with because it's an elegant piece of protocol design:

#### Loop prevention without a distributed computation
Distance-vector protocols like RIP need count-to-infinity mitigations (split horizon, poison reverse, hold-down timers) because a router only knows "next hop + cost," not the full path — so loops can form and are only detected indirectly, by watching a metric degrade. BGP sidesteps this entirely: if a router receives an advertisement whose AS_PATH already contains its own ASN, it silently discards it. The loop is visible directly in the data, not inferred from a metric. This is why path-vector protocols don't need the timer-based hacks that plague distance-vector protocols.

#### A crude but effective distance metric
AS_PATH length (number of AS hops) is one of the tie-breakers in BGP's best-path algorithm — but note the emphasis on "crude": it counts autonomous systems, not routers, not latency, not bandwidth. A 3-AS-hop path could easily be higher latency than a 4-AS-hop path if one of those ASes spans a transcontinental fiber run. AS_PATH length is a low-priority tie-breaker, not the primary decision criterion — policy attributes (§6) are evaluated first.


Terminology precision
Don't confuse AS_PATH with a real physical path. Multiple physical router hops exist inside each AS along the way; AS_PATH abstracts all of that away into one entry per AS. It also doesn't reveal which router inside a transit AS your packets will egress through — that's determined by that AS's own internal routing (IGP + iBGP, §5) and is invisible to you.



---

#### Loop Detection

Suppose your AS number already appears in the advertised path.

```text
AS100 → AS200 → AS300 → AS100
```

This would create a routing loop.

The router simply discards the route.

Loop detection becomes extremely easy because the complete path is explicitly carried in every advertisement.

---

#### Route Withdrawal

If a network becomes unreachable, the advertising router sends a **withdrawal message**.

Neighboring routers remove the route from their routing tables and select alternative paths if available.

---
## Sessions & the BGP Finite-State Machine

BGP is unusual among routing protocols in that it does not define its own transport — it runs directly over TCP port 179. This is a deliberate design choice: by outsourcing reliability, segmentation, and flow control to TCP, BGP's own message format can stay comparatively simple, and it inherits TCP's congestion control for free. Two routers that wish to exchange routes are called BGP peers (or neighbors), and they form one full TCP connection between them — a BGP session rides on top of it exactly like an HTTP or SSH session would.

### The four message types
| Message	 | Purpose	| Sent  |
|------------|----------|-------|
| OPEN	| Establish a BGP session.  Negotiate session parameters: ASN, hold-time, BGP version, capabilities |	Once, at session establishment|
| UPDATE	|  Advertise/Announcing new routes (NLRI + path attributes) and/or withdraw old ones	|Whenever routing information changes|
| KEEPALIVE	| Handshake at regular intervals.  Empty message confirming the peer is alive | Periodically (default 1/3 of hold-time, often 30s / 90s hold)|
| NOTIFICATION	| Report an error and close the session	|On protocol violation or dministrative shutdown | 

```
BGP announcement = prefix + path attributes

Path attributes 
Include: next hop, AS Path, local preference, Multi-­‐exit discriminator, ... 
Used to select among multiple options for paths.  
```

### The session finite-state machine 

Just as CS144's TCPConnection has a state machine you implement explicitly (LISTEN → SYN_RCVD → ESTABLISHED → ...), BGP peering has its own FSM defined precisely in RFC 4271. Understanding it matters operationally — "why is my BGP session stuck in Active?" is a real, common debugging question.


---


# IP Multicast

## Why multicast exists
Traditional Internet communication is **unicast**. One sender communicates with one receiver.

However, some applications need to send identical data to many receivers simultaneously.

Examples include:

* IPTV
* Live sports broadcasts
* Stock market feeds
* Video conferencing
* Online classrooms

Sending a separate copy to every receiver wastes bandwidth.

Multicast solves this problem.


Start with the problem unicast can't solve efficiently. Suppose a video stream needs to reach 10,000 subscribers. With pure unicast, the source (or some replication point) must send **10,000 separate copies** of every packet — bandwidth cost scales linearly with receiver count, even though the vast majority of that traffic is byte-for-byte identical.

**Multicast's core idea:** let the *network* replicate the packet, not the sender. The sender transmits **one copy** addressed to a *group*, and routers along the way duplicate it only at points where the delivery tree actually branches toward multiple interested receivers.

> **Precision point:** multicast is a *network-layer* (IP) concept — distinct from Ethernet broadcast (link-layer, all hosts on a segment) and from application-layer multicast (overlay networks doing replication in software, not in routers). This section is specifically about **IP multicast**, where routers themselves build and maintain a delivery tree.


## Techniques and Principles 
- Reverse Path Broadcast (RPB) and Pruning 
- One versus multiple trees Practice 
- IGMP – group management 
- DVMRP – the first multicast routing protocol 
- PIM – protocol independent multicast 



### 1. Building the delivery tree: Reverse Path Broadcast (RPB) + Pruning
RPB + Pruning 
1. Packets delivered loop-­‐free to every end host. 
2. Routers with no interested hosts send prune 
messages towards source. 
1. Resulting tree is the minimum cost spanning tree 
from source to the set of interested hosts. 


This is the mechanically interesting part. Given that hosts join groups locally via IGMP, how do routers cooperate to build a loop-free, efficient tree from the source to every interested receiver, without any router needing a global view of who's subscribed?

**Step 1 — Reverse Path Broadcast (RPB).** Every router already has a unicast routing table telling it the shortest path *back toward* any given source (this is exactly the "reverse path" — the path packets would take if flowing *from* this router *to* the source). The RPB rule is:

> A router forwards a multicast packet out on all interfaces *except* the one it arrived on, **but only if the packet arrived on the interface that is on the shortest path back to the source.**

If the packet arrives on any *other* interface, it's dropped. This single rule guarantees loop-freedom without any router needing to compute or store an explicit tree — each router makes a purely local decision based on information it already has (its own unicast reverse-path routing entry for that source).

```text
                     Source
                       |
                 (shortest path back to source is via this link)
                       |
                    Router A
                   /    |    \
              Router B  |   Router C     <- A forwards to B, C (correct reverse path)
                 |      |
              Router D  |
                 |      |
              (duplicate arrival at D from two paths:
               only the copy on D's shortest-path-back interface is forwarded onward)
```

The consequence of RPB alone: packets flood to **every router** in the network, whether or not anyone downstream actually wants the traffic. That's correct (loop-free, reaches everyone) but wasteful.

**Step 2 — Pruning.** A router with **no interested hosts on any of its downstream branches** — no local IGMP membership, and no downstream router has told it otherwise — sends a **prune** message back toward the source, telling its upstream neighbor "stop sending me this group's traffic." The upstream router marks that interface as pruned and stops forwarding onto it, *unless* a subsequent IGMP join reactivates it.

```text
Router D has no locally-joined hosts, and its own downstream
routers have already pruned it too:

  Router D  --- prune(group G) --->  Router B

Router B now forwards G only toward branches that still have
interested receivers downstream.
```

**Result:** after the prune messages propagate, what remains is exactly the **minimum-cost spanning tree** from the source to the current set of interested hosts — every router still receiving traffic is on a path to at least one subscriber, and no router receives traffic it has no use for.

> **Design tradeoff to notice:** RPB + Pruning is a *flood-and-prune* strategy — it's simple and requires no group-specific state until traffic actually starts flowing and pruning happens, but it means every new group briefly floods the entire network before pruning converges, and prune state itself has to be periodically refreshed (like IGMP's soft state) or re-flooded, since pruned branches don't remember "no interest" forever.



---

### 2. Group addresses — mechanism

**Group addresses** solve an *addressing* problem: how do you name "a set of receivers" with a single IP address, when normal IP addresses name exactly one interface? A unicast address is a 1:1 name. You need a mechanism where one address can mean "however many hosts currently care about this," a set whose membership changes at any time, with no central registry of who's in it.

#### 2.1 The address range

IPv4 carves out an entire class for this: **224.0.0.0 – 239.255.255.255**, i.e. **224.0.0.0/4** — historically called Class D. Structurally, this isn't a network/host split the way a normal IPv4 address is; the top 4 bits (`1110`) simply say "this is a multicast address," and the remaining 28 bits identify the group.

```text
Class D address, e.g. 224.1.1.1:

1110 0000 . 0000 0001 . 0000 0001 . 0000 0001
^^^^
"this is multicast" marker (fixed top nibble)
     \_______________________________________/
              28 bits: group identifier
```

#### 2.2 There is no "owner" — and that's a deliberate design choice, not an oversight

A unicast address is administratively assigned to exactly one interface (by DHCP, static config, whatever). A group address has **no such assignment step**. Any host can send to any group address at any time; any host can join any group at any time. There's no registry saying "224.1.1.1 belongs to Netflix." This is intentional: multicast group membership is meant to be as dynamic and cheap as a TV channel — you tune in, you tune out, no permission needed, no central coordination required per-join.

The tradeoff this creates (worth flagging honestly, not glossing over): **there is no built-in access control at the addressing layer.** Anything can send to a group; anything can listen. Real deployments layer authorization on top (application-layer encryption, source-specific multicast restricting which sources are valid for a group — more on that below) rather than relying on the address space itself to gatekeep.

#### 2.3 Mapping a Class D address onto an Ethernet multicast MAC address

This is the part that's genuinely subtle and worth being precise about, because it's a real source of bugs in practice.

Ethernet has its own multicast addressing convention: any destination MAC with the **low-order bit of the first byte set to 1** is treated by a NIC as a multicast/broadcast frame rather than "check if this matches my unicast address." IANA owns a reserved block of Ethernet multicast addresses for IP multicast specifically:

```text
01:00:5E:00:00:00  through  01:00:5E:7F:FF:FF
```

That's 25 bits of usable space in the Ethernet address (`01:00:5E` fixed, then the top bit of the next byte fixed to 0, leaving 23 bits free). The mapping rule takes the **low-order 23 bits** of the IP multicast address and places them directly into those 23 free bits of the MAC address:

```text
IP:  224.  1.  1.  1
     1110 0000 . 00000001 . 00000001 . 00000001
                     \____________23 low bits___________/
                          (drop the high 5 bits entirely)

MAC: 01:00:5E:01:01:01
     01:00:5E : 0 0000001 : 00000001 : 00000001
                ^ high bit of this byte always 0
```

**The critical consequence of dropping those 5 high-order bits: the mapping is not one-to-one.** There are 28 bits of IP group-address space, but only 23 bits make it into the Ethernet address — so **32 different IP multicast groups map onto the exact same Ethernet multicast MAC address.** Concretely, 224.1.1.1, 224.129.1.1, 225.1.1.1, and 29 other IP groups all collide onto `01:00:5E:01:01:01`.

> **Protocol-level implication, not a CS144 simplification:** a NIC filtering purely on the Ethernet destination address cannot distinguish these 32 groups from each other. A host that joined group A but not group B may still have B's frames delivered up to the IP layer by the NIC hardware filter, simply because they alias to the same MAC. IP itself must perform the final, exact filter — checking the actual 32-bit destination IP address in the IP header — before deciding whether to accept the packet. This is a real, occasionally-encountered gotcha in low-level driver/NIC-filtering debugging: "why is my socket receiving traffic for a group I never joined?" is very often exactly this 32:1 aliasing.

---

### 3. IGMP — mechanism

IGMP: Internet group management protocol
- Between host and directly attached router. 
- Hosts ask to receive packets belonging to a particular multicast group. 
- Routers periodically poll hosts to ask which groups they want. 
- If no reply, membership times out (sod-­‐state). 


**IGMP** solves a completely different, *local signaling* problem: given that a group address exists, how does a host on a LAN tell its directly-attached router "deliver that group's traffic to me"? And just as important: how does the router find out when nobody on that LAN cares anymore, without requiring every host to reliably say "goodbye" first?

Keep these two questions distinct as we go — group addresses are a **naming/addressing** mechanism (lives at the IP layer, global in scope); IGMP is a **local membership protocol** (lives between host and first-hop router, link-local in scope, never crosses a router boundary into the wider multicast distribution tree).

#### 3.1 Where IGMP sits, precisely

IGMP is carried directly inside IP packets (protocol number 2), **not** over UDP or TCP — it is itself a network-layer control protocol, structurally analogous to ICMP in that sense (in fact IGMP's message format closely mirrors ICMP's).

```text
                +-------------------+
Application     |                   |
                +-------------------+
Transport       |    TCP / UDP      |
                +-------------------+
Network         |  IP  |   IGMP     |   <- IGMP is a peer of IP, not riding on top of UDP/TCP
                +-------------------+
Link            |     Ethernet      |
                +-------------------+
```

And critically: **IGMP is scoped to a single link.** It runs strictly between a host and its directly-attached router (or, more precisely, the router acting as "querier" on that LAN segment — see below). It never propagates beyond the first-hop router. The router then uses a *separate* protocol (DVMRP/PIM, as covered in the earlier tutorial) to actually build the multicast delivery tree across the wider network based on what IGMP told it locally.

### 3.2 The message types

| Message | Sent by | Purpose |
|---|---|---|
| **Membership Query** | Router | "Who on this segment still wants which groups?" — sent periodically, and can target a specific group or ask generally |
| **Membership Report** | Host | "I want group G" — sent unsolicited when first joining, and sent in response to a Query to reaffirm interest |
| **Leave Group** (IGMPv2+) | Host | "I no longer want group G" — an explicit departure signal, not present in IGMPv1 |

#### 3.3 The join/query/report cycle, precisely timed

```text
Host                                    Router (querier)
  |                                            |
  | ---- Membership Report (join G) --------> |   host wants to receive G
  |                                            |   router starts forwarding G onto this link
  |                                            |
  |             ... time passes ...            |
  |                                            |
  | <---- General Query (periodic) ----------- |   "which groups does anyone here still want?"
  |       (sent to 224.0.0.1, "all hosts")      |
  |                                            |
  | ---- Membership Report (still want G) ---> |   reaffirm before timeout
  |                                            |
  |             ... query interval repeats ...  |
```

The router sends General Queries periodically (default interval commonly 125 seconds in IGMPv2/v3 deployments — the exact number is a tunable, not a protocol-fixed constant). Every host that still wants a group must respond before that group's membership state on the router times out.

#### 3.4 Soft state — why this design, and exactly what it buys you

This is the single most important conceptual point in IGMP, and it's worth deriving *why* rather than just stating it.

**The naive alternative** would be hard state: host sends Join once, router remembers it forever, host must send an explicit Leave to remove it. This is simpler on the wire but fragile in exactly the way distributed systems always are fragile around explicit teardown: what happens if the host crashes, loses power, or its network interface goes down uncleanly? The Leave message never gets sent. The router now believes a host still wants group G indefinitely, forwarding traffic onto a link where nothing is listening — a permanent, undetectable inconsistency between real state and protocol state.

**Soft state inverts the default.** Membership is a *lease*, not a fact recorded once and trusted forever:

- A host's membership is only valid until the next Query cycle, unless refreshed.
- The router's assumption, implicitly, is "prove to me periodically that you're still there" rather than "tell me once and I'll believe it until told otherwise."
- If a host disappears ungracefully, the worst case is a bounded delay — one or two missed Query intervals — before the router correctly concludes no one wants G anymore and stops forwarding. There's no permanently-wrong state; the system self-heals by construction.

> **This is the general soft-state design pattern** you'll see recur elsewhere in networking (DHCP leases work the same way — a lease you must renew, not a permanent grant) — trading a small bounded staleness window for the elimination of an entire class of "crash left me in a stuck state" bugs. It's the same underlying engineering tradeoff as TCP's use of timers/retransmission instead of assuming reliable delivery: assume the last message might not arrive, design for that assumption from the start, rather than bolting on error recovery after the fact.

#### 3.5 Query targeting and Report suppression — an efficiency mechanism worth knowing

A subtlety real IGMP deployments care about: if 50 hosts on a segment all want group G, you don't want all 50 to respond to every Query — that's needless traffic, and the router only needs to know "at least one host still wants G," not the full membership list.

IGMP handles this with a **randomized response timer**: on receiving a Query, each interested host doesn't reply immediately — it starts a random timer (bounded by a Max Response Time field the Query itself carries) and only sends its Report when that timer expires. But Reports are sent to the **group's own multicast address**, meaning every other host interested in that same group also receives it. If a host sees another host's Report for a group it was also about to report, it **cancels its own pending Report** — one confirmation per group per segment is sufficient for the router's purposes. This is a real efficiency mechanism (this specific behavior is present in IGMPv1/v2; IGMPv3 changes some of the addressing details but keeps the core suppression idea), not a CS144 simplification — the intent is explicitly to avoid an O(n) reply storm to every periodic query.

### 3.6 Querier election — who's actually allowed to send Queries?

On a LAN with multiple routers attached (common for redundancy), only **one** router should be sending Queries — otherwise hosts get duplicate, possibly conflicting query traffic. IGMPv2 defines a simple election: every multicast-capable router on the segment listens for other routers' Queries; the router with the **lowest IP address** on the segment becomes the querier, and all others suppress their own Query transmission (falling back to sending Queries themselves only if the current querier's Queries stop arriving — detected via a timeout, again a soft-state pattern). This is genuinely analogous in spirit to the STP root election you already worked through: total ordering on an ID, lowest value wins, and everyone defers to whoever currently holds that role until proven otherwise by silence.

### 3.7 IGMP versions — what actually changed, and why it matters

| Version | Key addition | Why |
|---|---|---|
| **IGMPv1** | Basic Join + Query only | No explicit Leave — membership timeout is the *only* departure mechanism, so leave latency is bounded by the full query interval |
| **IGMPv2** | Explicit **Leave Group** message | Lets a host proactively signal departure, so the router can stop forwarding much sooner than waiting out a full query timeout — but note this is an *optimization* layered on top of soft state, not a replacement for it; the router still must handle a host that never sends Leave |
| **IGMPv3** | **Source filtering**: a host can join a group but specify *which sources* it wants (`INCLUDE {S1, S2}`) or explicitly exclude (`EXCLUDE {S3}`) | This is the mechanism underpinning **Source-Specific Multicast (SSM)** — instead of "give me anything sent to group G from anyone," a host can say "give me G, but only from this specific source IP," which closes part of the "anyone can send to a group" security gap noted in §2.2 |

> **Precision on scope:** CS144's treatment (per your uploaded slides) covers the conceptual host↔router Join/Query/soft-state model — this is squarely IGMPv1/v2 territory. IGMPv3 source filtering is real, deployed, operationally important (SSM is the basis of most modern IPTV deployments specifically *because* it removes the "any source can inject traffic into my group" ambiguity), but it's an extension beyond what the lecture slides describe — flagging it as additional context, not something the course expects you to derive from the given material.

---

### 4. Implementation-level detail: what actually happens on a real Linux box

Since sockets are in scope for this project — when a C++ program calls `setsockopt()` with `IP_ADD_MEMBERSHIP` (or the modern `MCAST_JOIN_GROUP`), it isn't just a userspace bookkeeping operation:

1. The kernel records that this socket is interested in the group on a given interface.
2. The kernel's IGMP implementation sends the actual **Membership Report** onto the wire — this is the userspace-triggered event that causes real IGMP traffic to appear.
3. The kernel also typically programs the NIC's **multicast filter** (a hardware or driver-level MAC filter table) to accept frames destined to the corresponding Ethernet multicast MAC — this is exactly where the 32:1 aliasing from §2.3 becomes an observable, real behavior: the NIC-level filter is coarser than the socket's actual IP-level interest.
4. The kernel is also responsible for periodically responding to Queries on the socket's behalf for as long as the socket remains joined — this is *not* something the application has to re-trigger manually; the soft-state refresh is handled transparently below the socket API.

Switches in between (if any) commonly implement **IGMP snooping** — passively watching IGMP traffic pass through, without participating as a router, purely so the switch can restrict which of its own switch ports actually receive a given group's frames, rather than falling back to flooding multicast to every port on the VLAN (the switch's default behavior for any destination MAC it has no more specific forwarding entry for, per the learning-bridge logic from the STP discussion). This is a link-layer, vendor/implementation-specific optimization — not part of the IGMP RFCs themselves, but near-universal in real switch hardware.

## Multicast routing protocols in practice

| Protocol | Approach | Notes |
|---|---|---|
| **DVMRP** (RFC 1075) | Distance-Vector Multicast Routing Protocol | The first Internet multicast routing protocol; directly implements RPB + Pruning using its own distance-vector routing exchange (not the unicast table) |
| **PIM** — Dense Mode (RFC 3973) | Protocol Independent Multicast | Conceptually similar to DVMRP's flood-and-prune, but "protocol independent" — it relies on whatever unicast routing protocol is already running (OSPF, BGP, etc.) for reverse-path checks, rather than maintaining its own routing exchange |
| **PIM — Sparse Mode** (RFC 4601) | Rendezvous-point based | Built for the common real-world case where interested receivers are a *small, sparse* subset of the network — flooding everywhere first (dense mode's approach) is wasteful when only a few routers actually want the traffic |

**Why sparse mode is structurally different, not just a tuning knob:** flood-and-prune assumes "most routers want this traffic, prune the exceptions" — a reasonable assumption for, say, a routing protocol's own control traffic distributed to all routers. But for something like an IPTV channel, the assumption inverts: most routers have *no* interested hosts. Flooding the whole network only to immediately prune 99% of it is pure waste. PIM sparse mode instead builds a **shared tree** rooted at a **Rendezvous Point (RP)** — receivers explicitly join toward the RP (an explicit "join," not "everyone gets it unless they prune"), and the source sends to the RP, which forwards down the shared tree. This flips the default from "on unless told otherwise" to "off unless explicitly requested," which scales far better when interest is sparse.

Two major multicast routing protocols have been used.

### DVMRP

DVMRP 
- Distance Vector Multicast Routing Protocol (RFC 1075) 
- First Internet routing protocol 
- Uses RPB + Prune 



The **Distance Vector Multicast Routing Protocol (DVMRP)** was the first Internet multicast routing protocol. It relies on Reverse Path Broadcasting followed by pruning to eliminate unnecessary branches.



### PIM
PIM 
- Protocol Independent Multicast 
- Two modes: dense mode, sparse mode 
- Dense mode (RFC 3973): Similar to DVMRP 
- Sparse mode (RFC 4601): Builds rendezvous points 
through which packets join small set of spanning trees. 

**Protocol Independent Multicast (PIM)** is the protocol most commonly used today.

It supports two operating modes:

* **Dense Mode**, which assumes receivers are widely distributed and behaves similarly to DVMRP.
* **Sparse Mode**, which assumes relatively few receivers and builds shared distribution trees through **Rendezvous Points (RPs)**, reducing unnecessary traffic.

Although multicast never became as widespread as originally expected—largely because Internet traffic is increasingly personalized (such as on-demand video streaming)—it remains important for IPTV, financial data distribution, and other specialized applications.

---

> **Key Takeaway:** Multicast efficiently delivers one stream of data to many receivers by constructing distribution trees instead of sending separate copies to each destination.



## Multicast in practice: why it's less common than expected

Multicast used less than originally expected 
- Most communication is individualized(e.g. time shiding) 
- Early implementations were inefficient 
- Today, used for some IP TV and fast dissemination 
- Some application-­‐layer multicast routing used 
Some interesting questions 
- How to make multicast reliable? 
- How to implement flow-­‐control? 
- How to support different rates for different end users? 
- How to secure a multicast conversation?


Multicast's designers expected it to become a default Internet-wide mechanism. It didn't, for a few concrete reasons:

- Most real communication turned out to be **individualized** rather than truly simultaneous-broadcast — e.g., video-on-demand and time-shifted viewing (the exact scenario multicast is best at — simultaneous live delivery to many receivers — is a shrinking fraction of total traffic).
- Early implementations were inefficient, and multicast routing requires router state and cooperation across administrative boundaries that most ISPs were reluctant to deploy and support.
- It remains genuinely useful for **live IPTV** and **fast dissemination** scenarios (e.g., financial market data feeds within a single administrative domain), where the simultaneous-many-receivers assumption actually holds.
- Where multicast-like behavior is wanted at Internet scale today, it's often built at the **application layer** instead (peer-assisted distribution, CDN fan-out) rather than relying on network-layer IP multicast support end-to-end.

**Open problems multicast raises that unicast doesn't have to deal with**, worth sitting with conceptually even without solving them here:
- **Reliability:** unicast TCP has one sender and one receiver negotiating retransmission; multicast has one sender and potentially thousands of receivers, each of which might need different packets retransmitted. There's no single well-agreed general solution.
- **Flow control:** whose receive window governs the sender's rate, when receivers have wildly different capacities?
- **Heterogeneous rates:** should all receivers get the same encoding, or can the network deliver different quality streams to different receivers (layered/scalable coding)?
- **Security:** who's authorized to join a group, and how do you encrypt a stream for a *dynamically changing* set of receivers without re-keying on every join/leave?

---

# The Spanning Tree Protocol (STP)

Ethernet “routes” packets too. 
We know how addresses are learned, but how are loops prevented? 
Ethernet switches build a spanning tree over which  packets are forwarded. 






## Why Ethernet needs its own loop-avoidance mechanism

### Ethernet Switch 
1. Examine the header of each arriving frame. 
2. If the Ethernet DA is in the forwarding table, forward the frame to the correct output port(s). 
3. If the Ethernet DA is not in the table, broadcast the frame to all ports (except the one through which the frame arrived). 
4. Entries in the table are **learned** by examining the Ethernet SA of arriving packets.  

Learning could lead to loops 


Ethernet switches "route" packets too, in a specific sense worth being precise about: **learning bridges**. A switch doesn't run a routing protocol like OSPF — it builds a MAC-address forwarding table purely by observation:

 

```text
Switch forwarding logic, per arriving frame:

1. Look up the frame's destination MAC (DA) in the forwarding table.
2. If found  -> forward out only that specific port.
3. If not found -> flood out every port except the one it arrived on.
4. Learn: record (source MAC of this frame, the port it arrived on)
          in the forwarding table, regardless of what step 2/3 did.
```

This is elegant and requires zero configuration — the table builds itself from traffic the switch already sees. But it has no built-in concept of *topology* — it assumes there's exactly one path between any two switches. **If the physical topology has a loop** (often deliberately, for redundancy — you don't want a single cable failure to partition your network), flooded frames circulate forever, duplicating and re-flooding at every switch in the loop. This is a **broadcast storm**, and it can saturate a LAN in well under a second.

We know how addresses are learned, but how are loops prevented? 
Ethernet switches build a spanning tree over which  packets are forwarded.

> **Precision point:** this is a link-layer problem, structurally analogous to (but mechanistically distinct from) the network-layer loop problem RPB solves for multicast above. Ethernet has no TTL/hop-count field in the classic sense to bound flooding the way IP has TTL — so an actual topological loop, not just a routing miscalculation, has to be physically prevented from ever being active for forwarding.


## The STP model: compute a loop-free subgraph

### Ethernet SA of arriving packets. 

Preventing loops Spanning Tree Protocol 
The topology of switches is a graph. 
The Spanning Tree Protocol finds a a subgraph that spans all the vertices without loops. 
- Spanning: all switches are included. 
- Tree: no loops. 
The distributed protocol decides: 
1. Which switch is the Root of the tree 
2. Which ports are allowed to forward packets along the tree.  


### How it works 

1. Periodically, all switches broadcast a “Bridge Protocol Data Unit” (BPDU) 
 (ID of sender, ID of root, distance from sender to root). 
2. Initially, every switch claims to be Root: sets distance field to 0. 
3. Every switch broadcasts until it hears a “be^er” message: 
- A root with a smaller ID 
- A root with equal ID, but with shorter distance 
- Ties broken by smaller ID of sender. 
4. If a switch hears a be^er message, retransmit message (add 1 to distance). 

Root port: The port on a switch that is closest to the Root. 
Designated port: The port neighbors agree to use to reach the Root.  
All other ports are blocked from forwarding (but still sendtireceive BPDUs). 

Eventually: 
- Only the root originates configuration messages (others retransmit them). 
- Locally, switch only forwards on ports. 


Model the network of switches as a graph — switches are vertices, links between them are edges. **The Spanning Tree Protocol finds a spanning subgraph with no cycles**:

- **Spanning** — every switch is included (no switch is left unreachable).
- **Tree** — by definition, exactly one path exists between any two vertices; no cycles.

The distributed algorithm has to answer two questions with no central coordinator: **(1) which switch is the root of the tree**, and **(2) which ports on each switch are allowed to actually forward frames** (the rest stay up for redundancy but blocked from forwarding, ready to activate if the topology changes).





### The algorithm: BPDUs and port roles

Every switch periodically broadcasts a **BPDU** (Bridge Protocol Data Unit) containing three fields: **(sender's ID, sender's believed root ID, sender's distance to that root)**.

```text
BPDU = { sender_id, root_id, distance_to_root }
```

**Initialization:** every switch starts by believing *it itself* is the root — it sets `root_id = its own ID` and `distance = 0`, and broadcasts that.

**Convergence rule:** a switch keeps re-broadcasting its current belief until it hears a strictly **better** claim from a neighbor, where "better" is a precise, total-ordered comparison:

1. A smaller root ID beats a larger one, *or*
2. Equal root ID, but a shorter distance to that root, *or*
3. Equal root ID and equal distance — broken by the smaller ID of the sender.

When a switch hears a better claim, it **adopts** that root ID, updates its own distance (its neighbor's distance + 1), and re-broadcasts the improved claim onward — so better root claims propagate outward from wherever they originate, and worse claims simply stop being re-broadcast once a switch sees something better.

**Port roles**, once the algorithm settles (every switch agrees on one root and its own distance to it):

| Port role | Meaning |
|---|---|
| **Root port** | The one port on a (non-root) switch that is closest to the Root — the port this switch would use to reach the root. Exactly one per non-root switch. |
| **Designated port** | On each link/segment, the port that neighboring switches agree to use to reach the Root through that segment — effectively "the switch closer to the root, on this shared link, wins the right to forward here." |
| **Blocked port** | Any port that is neither root nor designated for its segment. It still sends/receives BPDUs (so it can detect topology changes and potentially reactivate), but it does **not** forward data frames. |

**The end state:** only the Root switch originates *new* configuration messages from scratch (claiming distance 0); every other switch's ongoing behavior is to **retransmit** the best message it's heard, with its own distance added. Locally, each switch forwards data frames only on its root port and any designated ports it holds — everything else is blocked. This is exactly the mechanism that turns an arbitrary, redundant physical graph into one active, loop-free logical tree, while keeping the redundant links physically present and ready to take over if a link or switch fails.

> **Why this matters operationally:** because blocked ports still exchange BPDUs, STP detects topology changes (a link or switch failing) and recomputes — a previously-blocked port can become a root or designated port if the path it was backing up is now needed. This is the entire value proposition of deploying physically redundant Ethernet topology in the first place; STP is what makes that redundancy safe to wire up without a permanent broadcast storm.

## A brief history: 


| Year | Development |
|---|---|
| 1985 | STP proposed; standardized as IEEE 802.1D in 1990. Still extremely widely deployed. |
| 2004 | Replaced (for switches that support it) by **RSTP** (Rapid STP) — same fundamental model, but converges far faster after a topology change. |
| — | Even RSTP has a structural inefficiency worth naming precisely: **it still only uses one active tree** — meaning only one path between any two switches is ever active for forwarding, even if the physical topology has many redundant links that could, in principle, be used for parallel active paths and load balancing. |
| 2012 | **Shortest Path Bridging (SPB, IEEE 802.1aq)** — a genuinely different approach: a **link-state** protocol (structurally like OSPF, at the link layer) that computes actual shortest paths between switches and can use multiple paths simultaneously, rather than collapsing the whole topology down to one spanning tree. |

---

# IPv6









## 1 Why IPv6 exists: the intuition is address exhaustion

Goal of Internet Protocol Addresses

- Stitch many different networks together
- Need network-independent, unique address
  - Well, these days it can be only mostly unique -- see NATs, anycast, etc.
- But there are only 232 IPv4 addresses
  - Generally, utilization is ~35%
  - Need an address to communicate..

IPv4 addresses are 32 bits — **2³² ≈ 4.3 billion** possible addresses. That sounds like a lot until you account for real-world utilization inefficiency: historical allocation practices (giving out entire /8 blocks to early adopters), the sheer growth in number of connected devices, and the fact that usable address space is rarely close to 100% efficiently assigned — actual global utilization has historically sat around roughly 35%. Combined with explosive growth in the number of devices needing globally-unique addresses, IPv4 space was on a clear trajectory toward exhaustion.

**Design goal, precisely stated:** IP addressing needs to give every device on every network a **network-independent, (at least practically) unique address** so that arbitrary networks can be stitched together into a single internetwork. That's the same underlying goal IPv4 was designed for — IPv6 doesn't change the goal, it changes the size of the address space to meet it at Internet scale for decades to come.

> **Caveat worth noting precisely, since it's easy to overstate:** even under IPv4, "unique" was already not strictly literal in practice — NAT lets many devices share one public address, and anycast lets one address correspond to many physical devices/locations by design. IPv6's larger space removes much of the *pressure* that made NAT necessary, but it doesn't retroactively make either NAT or anycast go away as concepts.

## A brief timeline


| Year | Milestone |
|---|---|
| 1994 | Work on the next-generation IP begins within the IETF. |
| 1998 | The basic IPv6 protocol is published as RFC 2460. |
| 2003–2006 | A lull in adoption, followed by renewed interest. |
| Today | Active, sustained push within the IETF and among operators for full deployment, as IPv4 exhaustion becomes operationally real rather than theoretical. |

## Address structure

- IPv6 has 128 bits of address
  - 2128 (3.4x1038) addresses: 21 addressestiin2 of the world’s surface!
- Separated into subnet and interface portions (RFC 4291)
- Write address in hexidecimal as 8 blocks of 16 bits, separated by :
  - market.scs.stanford.edu: 2001:470:806d:1::9 prefixlen 64
  - Can omit a single run of zeros with ::
  - Use brackets in URLs: http:titi[2001:470:806d:1::9]:80
  - Can write low 32 bits like IPv4: 64:ff9b::171.66.3.9
- 
IPv6 addresses are **128 bits** — 2¹²⁸ (≈3.4 × 10³⁸) possible addresses. The scale is easiest to make concrete with the comparison from the original material: that's roughly enough addresses to assign **2¹⁵ addresses to every square meter of the Earth's surface** — the design goal was explicitly to never need to revisit this exhaustion problem again, not just to buy a few more decades.

Per **RFC 4291**, an address is structurally split into a **subnet portion** and an **interface portion** (conceptually parallel to network/host splitting in IPv4, though the mechanics and typical split point, /64 for the interface portion in most deployments, differ).

**Textual representation** — addresses are written in hexadecimal, as **8 groups of 16 bits each**, separated by colons:

```text
2001:0470:806d:0001:0000:0000:0000:0009
```

Two compression rules make this practical to actually read and write:

1. **Leading zeros within a group can be omitted:** `0001` → `1`, `0000` → `0`.
2. **A single run of consecutive all-zero groups can be collapsed to `::`** — but only once per address, since otherwise the expansion would be ambiguous (there'd be no way to tell how many zero groups each `::` represents).

Applying both rules to the example above:

```text
2001:470:806d:1::9
```

which is exactly the example given for `market.scs.stanford.edu`, with a `/64` prefix length.

**In URLs**, an IPv6 address must be wrapped in brackets to disambiguate the address's own colons from the URL's port-separator colon:

```text
http://[2001:470:806d:1::9]:80
```

**Embedding an IPv4 address in the low 32 bits** is also supported directly in the text notation — useful for IPv4-mapped or translated addresses:

```text
64:ff9b::171.66.3.9
```

Here the low 32 bits are written in familiar IPv4 dotted-decimal form rather than being converted to two more hex groups — both notations describe the same 128-bit value, but the mixed form is far more readable when the low bits specifically encode an IPv4 address (this particular prefix, `64:ff9b::/96`, is the well-known prefix used for algorithmic IPv4/IPv6 translation, NAT64).

---


