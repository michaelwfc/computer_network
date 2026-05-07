# Finite State Machines & Protocol Specification

*CS144 · Stanford University · A guide for junior developers*

---

## Introduction

Network protocols like TCP aren't just clever algorithms — they are precise contracts written in a formal language. This tutorial teaches you that language: Finite State Machines. By the end, you'll understand how the designers of the Internet describe complex, multi-party communication as a set of states and transitions, and you'll be able to read — and reason about — the famous TCP state diagram yourself.

---

## Section 01 — Why Precise Specification Matters

Imagine two engineers on opposite sides of the world, each independently building their half of a network protocol. One writes the client, the other writes the server. When their code finally meets on the wire, it either works perfectly or fails in mysterious ways — and there is no easy debug session they can share. The only thing connecting them is a **specification**: a written description of exactly how the system should behave in every possible situation.

Plain English is surprisingly bad at this job. Sentences are ambiguous. Paragraphs leave edge cases unaddressed. Readers make different assumptions. What you need is a formal notation that is precise enough to be read the same way by every implementer, yet visual enough to be understood at a glance. That notation is the **Finite State Machine**, or FSM.

FSMs appear everywhere in computing — in compilers, game logic, UI workflows, and network protocols. But they're especially central to **networking**, where many parties must agree on behavior without being able to negotiate in real time. The TCP protocol — the backbone of virtually all web traffic — is formally defined by an FSM with 12 states. By learning FSMs, you're gaining the ability to read and reason about the same specification documents that built the Internet.

> **Key Takeaway**
> Protocol specifications must be unambiguous because independent teams implement both sides of a conversation. Finite State Machines provide a formal, visual language that eliminates the vagueness of plain English and leaves no room for misinterpretation.

---

## Section 02 — What Is a Finite State Machine?

An FSM is exactly what its name suggests: a machine that can be in one of a **finite number of states**. A **state** is a snapshot — a particular configuration of the system at a point in time. Think of a vending machine: it might be in the "waiting for coins" state, the "coin inserted" state, or the "dispensing item" state. At any moment, it is in exactly one of those states, never two at once.

The word "finite" is doing important work here. A vending machine doesn't have an infinite number of modes — it has a bounded, enumerable list of them. This boundedness is what makes FSMs useful for specification: you can exhaustively describe every situation the system might encounter.

In diagrams, states are typically drawn as **circles or rounded rectangles**, each labeled with a name. An abstract three-state FSM looks like:

```
[ State 1 ] ──→ [ State 2 ] ──→ [ State 3 ]
```

The system is always in exactly one state. This mutual exclusivity is what makes FSMs precise: there is no ambiguity about "where" the system is.

> **Key Takeaway**
> A Finite State Machine describes a system using a bounded list of states — configurations the system can occupy. At any instant, the system is in exactly one state. This mutual exclusivity is what makes FSMs precise: there is no ambiguity about "where" the system is.

---

## Section 03 — States, Transitions, Events, and Actions

States alone are just a list of positions. What makes an FSM *dynamic* — what gives it the ability to describe behavior over time — is the concept of a **transition**. A transition is an edge (an arrow) drawn between two states. When the system is in a given state and a specific **event** occurs, the transition fires: the system moves from the source state to the destination state.

Think of it like a traffic light: "green" is a state. The event "timer expires" triggers a transition to "yellow." The event "timer expires again" triggers a transition to "red." No ambiguity, no negotiation — just a deterministic rule.

### Anatomy of a Transition Arrow

```
                event causing state transition
                ──────────────────────────────
[ State A ] ──────────────────────────────────→ [ State B ]
                actions taken on transition
                (optional)
```

Each edge in an FSM diagram carries two pieces of information:

**Event (trigger):** The condition that causes the transition. This is written above the arrow. An event might be a message arriving, a timer firing, or a user calling a function — something that happens to the system from the outside.

**Action (response):** What the system *does* when the transition fires. This is written below the arrow. Actions are optional — not every transition has a side effect. But when an action exists, specifying it is critical. An incomplete specification that omits actions leaves implementers guessing, and guessing produces bugs.

### Multiple Transitions and Determinism

A state can have more than one outgoing transition — one for each event it might receive. The rule, however, is strict: **for any given state and any given event, there must be exactly one transition**. If the same event could lead to two different destination states from the same source, the FSM is *ambiguous* — the system literally cannot know what to do.

What happens if the system is in a state and an event arrives for which no transition is defined? The behavior of the FSM is **undefined**. This is both a design warning and a practical concern: undefined behavior in a network protocol means that different implementers will handle the situation differently, leading to interoperability failures.

> **Key Takeaway**
> Transitions connect states through events and optionally trigger actions. Every transition for a given (state, event) pair must be unique — ambiguity breaks the specification. Missing transitions leave behavior undefined, which in network protocols means unpredictable interoperability failures.

---

## Section 04 — FSM in Practice: An HTTP Request

Abstract principles only go so far. Let's ground the FSM concepts in something you interact with every day: loading a web page via HTTP. While a production HTTP client involves far more complexity, this simplified three-state model captures the essential structure.

### The Three States

Our HTTP client FSM has three states:

- **Idle** — viewing the current page or doing nothing
- **Page Requesting** — a connection is open and resources are being requested
- **Request Pending** — a single HTTP GET has been sent and we're awaiting the server's response

```
                  load new page / open connection
                  ─────────────────────────────▶
[ Idle ] ────────────────────────────────────────── [ Page Requesting ]
          ◀─────────────────────────────────────────
                  connection closed

                  more resources / HTTP GET
                  ─────────────────────────────▶
[ Page Requesting ] ──────────────────────────────── [ Request Pending ]
                    ◀─────────────────────────────
                    receive response
```

### Walking Through the States

The journey begins in **Idle**. When the user clicks a link (event: *load new page*), the FSM transitions to **Page Requesting** and takes the action of opening a TCP connection to the web server. Now in the Page Requesting state, if there are more resources to fetch (event: *more resources to request*), the FSM sends an HTTP GET (action) and moves to **Request Pending**. When the response arrives (event: *receive response*), it transitions back to Page Requesting, ready to fetch the next resource. When the connection closes, it returns to Idle.

### The Incompleteness Problem

This FSM has four events: *load new page*, *more resources*, *receive response*, and *connection closed*. Combined with three states, that's up to twelve possible (state, event) pairs. Our diagram only defines six of them. What should happen when *connection closed* fires while we're in **Request Pending**? Or when *receive response* arrives while we're in **Idle**?

This is the core tension in protocol specification. A fully explicit FSM — one that handles every (state, event) pair — can grow extremely large and intimidating. In practice, there are two approaches:

**Option 1 — Supplement with text:** Write down the common cases in the diagram and address edge cases in accompanying prose. This keeps the diagram readable while ensuring nothing is truly undefined.

**Option 2 — Intentional underspecification:** The Internet Engineering Task Force (IETF) — the body that designs Internet standards — sometimes deliberately leaves parts of a protocol undefined. The philosophy is that by specifying only what's necessary for interoperability, you preserve flexibility for future experimentation. If an edge case turns out to matter, a later revision of the specification can define it.

> **Key Takeaway**
> Real FSMs face a tradeoff between completeness and readability. Fully specifying every (state, event) pair is rigorous but creates overwhelming diagrams. In practice, protocol designers often specify only the common paths and handle edge cases in supplementary text — a pragmatic approach that the IETF uses deliberately to leave room for future refinement.

---

## Section 05 — The TCP State Machine: The Internet's Most Famous FSM

TCP — the Transmission Control Protocol — is the protocol underneath virtually every HTTP request your browser makes. It's responsible for ensuring that data arrives reliably, in order, and without corruption. Managing a TCP connection involves carefully coordinating two machines through a precisely specified sequence of states. That specification is a finite state machine with 12 states.

At first glance the TCP FSM looks intimidating. Twelve states, dozens of transitions, a mix of user-program events and network-packet events — it's a lot to take in at once. But it has a clean four-part structure:

| Part | States | Purpose |
|------|--------|---------|
| Connection Opening | `CLOSED`, `LISTEN`, `SYN_SENT`, `SYN_RECEIVED` | Establishing a new connection (the 3-way handshake) |
| Data Transfer | `ESTABLISHED` | Sending and receiving data |
| Connection Closing | `FIN_WAIT_1`, `FIN_WAIT_2`, `CLOSING`, `TIME_WAIT`, `CLOSE_WAIT`, `LAST_ACK` | Graceful teardown |
| Final State | `CLOSED` | Connection fully closed; node may forget it |

Notice that `CLOSED` appears at both the top and bottom of the diagram. This isn't a mistake — it reflects the lifecycle of a connection: every connection begins from nothing (CLOSED) and eventually returns to nothing (CLOSED). The FSM is a cycle.

TCP also introduces an important concept: a distinction between **passive openers** (servers) and **active openers** (clients). Both sides use the same FSM diagram, but they enter it from different starting transitions and traverse different paths through the opening handshake. One FSM, two perspectives — this elegance is one of TCP's great design achievements.

> **Key Takeaway**
> TCP's 12-state FSM defines every aspect of connection lifecycle — opening, data transfer, and closing — for both endpoints simultaneously. Despite its apparent complexity, it divides cleanly into four logical parts. Learning to read it section by section, rather than all at once, reveals the underlying clarity of the design.

---

## Section 06 — TCP Connection Opening: The Three-Way Handshake

Before two machines can exchange data over TCP, they need to **synchronize** — agree on starting sequence numbers, confirm that both sides are ready, and establish the connection. This is done via the **three-way handshake**: a sequence of three packets (SYN → SYN/ACK → ACK) that you'll see referenced throughout networking literature.

Think of it like a phone call: you ring someone (SYN), they pick up and say "hello?" (SYN/ACK), and you say "hi, I can hear you" (ACK). Only after that exchange does the conversation begin. The handshake confirms that both parties are reachable and ready.

### The Server Side: Passive Opener

The server is the **passive opener** — it waits for incoming connections rather than initiating them. When a server program calls `listen()` on a socket, the socket transitions from `CLOSED` to `LISTEN`. No packets are sent; this is purely a local state change. The server is now watching for incoming connection requests.

**Setting up the LISTEN state**

```javascript
// When a server calls listen(), the socket transitions:
// CLOSED → LISTEN
// No packets are sent — this is a local state change only.

const net = require('net');
const server = net.createServer();

server.listen(8080, () => {
  // Socket is now in LISTEN state.
  console.log('Server listening on port 8080');
});
```

*Calling `listen()` transitions the socket from CLOSED to LISTEN. No messages are sent over the network — this transition only changes local state, which is why its action is empty in the FSM diagram.*

### The Client Side: Active Opener

The client is the **active opener** — it initiates the connection. When the client calls `connect()`, two things happen simultaneously: the local socket transitions from `CLOSED` to `SYN_SENT`, and a SYN packet is sent over the network to the server's port.

**Initiating the handshake from the client**

```javascript
// When a client calls connect(), the socket transitions:
// CLOSED → SYN_SENT  (action: send SYN packet)

const net = require('net');
const client = net.createConnection({ port: 8080 }, () => {
  // By the time this callback fires, we're in ESTABLISHED.
  // The handshake completed successfully.
  console.log('Connected! SYN → SYN/ACK → ACK handshake complete.');
});
```

*In Node.js, `createConnection()` initiates the three-way handshake internally. The callback fires only when the handshake is complete and the socket reaches the ESTABLISHED state.*

### Walking the Handshake State by State

**Three-way handshake: complete state trace**

```javascript
// Initial state for both sides:
// Client: CLOSED    Server: CLOSED

// Step 1 — Server calls listen():
// Server: CLOSED → LISTEN   (no packets sent)

// Step 2 — Client calls connect() and sends SYN:
// Client: CLOSED → SYN_SENT (sends SYN)

// Step 3 — Server receives SYN, sends SYN/ACK:
// Server: LISTEN → SYN_RECEIVED (sends SYN/ACK)

// Step 4 — Client receives SYN/ACK, sends ACK:
// Client: SYN_SENT → ESTABLISHED (sends ACK)
// Client can now send data. ✓

// Step 5 — Server receives ACK:
// Server: SYN_RECEIVED → ESTABLISHED (no packet sent)
// Server can now send and receive data. ✓
```

*This state trace maps exactly to the TCP FSM diagram. Notice the asymmetry: the client reaches ESTABLISHED one step before the server does, because the client sends the final ACK and doesn't need to wait for confirmation that it arrived.*

> **Key Takeaway**
> The three-way handshake is a precise FSM traversal: both client and server move through specific states triggered by specific events (connect, SYN, SYN/ACK, ACK). The FSM makes explicit what a verbal description leaves vague — exactly when each side considers the connection established and ready for data.

---

## Section 07 — TCP Connection Teardown: A Careful Goodbye

Closing a TCP connection is significantly more complex than opening one — and understanding why reveals something deep about the protocol's design. Unlike the symmetric handshake, connection teardown is **asymmetric**: one side might want to stop sending data while the other still has data in-flight. TCP handles this gracefully by allowing each direction of the connection to be closed independently.

Where connection setup used **SYN** (synchronize) packets, teardown uses **FIN** (finish) packets. The side that sends the first FIN is called the **active closer**; the side that receives it first is the **passive closer**.

### The Active Closer's Path

When a program calls `close()` on a connected socket (from ESTABLISHED), it becomes the active closer and enters `FIN_WAIT_1`. A FIN packet is sent to the other side. From FIN_WAIT_1, three things can happen:

**Path A — Full close:** The passive closer acknowledges the FIN *and* sends its own FIN in the same response. The active closer moves directly to `TIME_WAIT`.

**Path B — Half-close:** The passive closer only acknowledges the FIN (it still has data to send). The active closer moves to `FIN_WAIT_2`, where it continues receiving data. When the passive closer eventually sends its own FIN, the active closer moves to TIME_WAIT.

**Path C — Simultaneous close:** Both sides call close at almost the same moment and send FINs to each other. Both enter `CLOSING`, and when each receives acknowledgement of its own FIN, both move to TIME_WAIT.

### The Passive Closer's Path

The passive closer receives a FIN and enters `CLOSE_WAIT`. It can continue sending data — the active closer will receive and acknowledge it. Only when the passive closer calls `close()` does it send its own FIN and enter `LAST_ACK`, waiting for the acknowledgement of that final FIN. When that ACK arrives, it transitions to CLOSED.

### Why TIME_WAIT Exists

Both closing paths converge at `TIME_WAIT`. Rather than immediately transitioning to CLOSED, TCP waits in TIME_WAIT for a period (typically twice the maximum segment lifetime, around 60–120 seconds). This delay is deliberate and important.

Imagine the final ACK the active closer sends gets lost in the network. The passive closer, stuck in LAST_ACK, would retransmit its FIN. If the active closer had already fully closed and freed its state, it couldn't respond — and the passive closer would be stuck forever. TIME_WAIT ensures the active closer remains available to handle that retransmission.

**Teardown: state trace for the half-close path (Path B)**

```javascript
// Active closer (e.g., the client calls close()):
// ESTABLISHED → FIN_WAIT_1   (sends FIN)

// Passive closer receives FIN:
// ESTABLISHED → CLOSE_WAIT   (sends ACK)

// Path B — half-close (passive still has data):
// Active:  FIN_WAIT_1 → FIN_WAIT_2 (receives ACK, no FIN yet)
// Passive continues sending data...
// Passive calls close():
// Passive: CLOSE_WAIT → LAST_ACK   (sends FIN)
// Active receives FIN:
// Active:  FIN_WAIT_2 → TIME_WAIT  (sends ACK)
// Passive receives ACK:
// Passive: LAST_ACK → CLOSED       ✓
// Active waits ~2×MSL, then:
// Active:  TIME_WAIT → CLOSED      ✓
```

*This is the half-close teardown path. The connection asymmetry — one side can still send after the other "closes" — is a deliberate TCP feature that allows streaming responses to finish gracefully even after the request is sent.*

> **Key Takeaway**
> TCP teardown is more complex than setup because connections are bidirectional — each direction must be closed independently. The FSM makes this explicit with six closing states that handle every combination: full close, half-close, simultaneous close, and the TIME_WAIT delay that prevents state-confusion when final ACKs get lost.

---

## Section 08 — Quiz: Test Your Understanding

The following two questions come directly from the CS144 lecture. For both questions, assume the TCP FSM diagram is the *only* documentation — there is no supporting text defining additional transitions. If a transition doesn't appear in the diagram, the behavior is undefined.

**Question 1**

> Suppose we start in the CLOSED state, then call `listen()`, then receive a SYN, then call `close()`. What state will we be in?
>
> Options: `CLOSED` · `SYN_SENT` · `SYN_RECEIVED` · `ESTABLISHED` · `FIN_WAIT_1` · `undefined`

<details>
<summary>Reveal Answer</summary>

**Answer: FIN_WAIT_1**

Trace the path: CLOSED → (call `listen()`) → LISTEN → (receive SYN, send SYN/ACK) → SYN_RECEIVED → (call `close()`, send FIN) → **FIN_WAIT_1**. The FSM has a defined edge from SYN_RECEIVED on the `close()` event — it leads to FIN_WAIT_1.

</details>

---

**Question 2**

> Suppose we start in the CLOSED state, then call `connect()`, then call `close()`. What state will we be in?
>
> Options: `CLOSED` · `SYN_SENT` · `SYN_RECEIVED` · `ESTABLISHED` · `FIN_WAIT_1` · `undefined`

<details>
<summary>Reveal Answer</summary>

**Answer: CLOSED**

Trace the path: CLOSED → (call `connect()`, send SYN) → SYN_SENT → (call `close()`) → **CLOSED**. The FSM has a defined edge from SYN_SENT on the `close()` event that returns directly to CLOSED, aborting the connection attempt.

</details>

---

These questions illustrate the practical power of FSM specifications: you don't need to guess what "should" happen in an edge case. You look up the state, find the event, and follow the arrow. If no arrow exists, the behavior is undefined — and that itself is an important piece of information for an implementer.

> **Key Takeaway**
> FSMs turn ambiguous "what happens if…" questions into deterministic lookups. Given a current state and an event, you either find a defined transition (and follow it) or you discover that the behavior is undefined — which is itself a meaningful specification outcome that implementers must handle.

---

## Summary: The Five Things to Remember

This tutorial covered a lot of ground — from abstract FSM structure to the full TCP lifecycle. Here are the five most essential takeaways:

**1. FSMs are the language of protocol specification.**
When you need two independent systems to agree on behavior without real-time negotiation, plain English fails. Finite State Machines provide a formal, visual notation where every state, event, and action is precisely defined and leaves no room for misinterpretation.

**2. A system is always in exactly one state.**
The mutual exclusivity of states is what makes FSMs deterministic. For any (state, event) pair, there is at most one defined transition — and if there isn't one, the behavior is explicitly undefined. Ambiguity in the diagram means bugs in the implementation.

**3. Completeness versus readability is a real tradeoff.**
Fully specifying every (state, event) pair is rigorous but produces overwhelming diagrams. Protocol designers — including the IETF — deliberately leave some transitions undefined to keep specifications manageable and to preserve design flexibility.

**4. TCP's 12-state FSM is the Internet's most important FSM.**
Despite its apparent complexity, it divides into four clean parts: connection opening (the 3-way handshake), data transfer (ESTABLISHED), connection teardown (6 states), and final closure (CLOSED). Understanding this structure makes the diagram readable, not intimidating.

**5. Connection teardown is harder than setup — intentionally so.**
TCP connections are bidirectional; each direction must be closed independently. The six closing states handle all cases: full close, half-close, simultaneous close, and the TIME_WAIT delay that prevents permanent state confusion when final ACKs are lost in transit.

---

*Based on CS144: Introduction to Computer Networking, Stanford University.*
