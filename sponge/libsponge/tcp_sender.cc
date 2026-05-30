#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <algorithm>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before
//! retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise
//! uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout,
                     const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()})),
      _initial_retransmission_timeout{retx_timeout}, _stream(capacity),
      //  constructed with initial RTO
      _retransmission_timer(retx_timeout) {}

// "Bytes in flight" means: sequence numbers you have sent but the receiver has
// not yet acknowledged. They are "in flight" — somewhere between sender and
// receiver, either still traveling through the network, lost, or received but
// not yet ACK'd. 
//Absolute sequence space: 
// 0        recv_base        next_seqno
// |             |                |
// ▼             ▼                ▼
// [  ACK'd  ][  IN FLIGHT  ][  not sent yet  ]
//             ^^^^^^^^^^^
//             bytes_in_flight()
// The Formula： bytes_in_flight() = _next_seqno - _last_acked_seqno
uint64_t TCPSender::bytes_in_flight() const {
  return _next_seqno - _last_acked_seqno;
}

// A loop to reads bytes from outgoing ByteStream and seands as many bytes as
// possible in the form of TCP segements, until the window is full or there are
// no more bytes to send. Sender must not send beyond what receiver announced.
// When receiver announces more space → sender fills it. This is the flow
// control contract of TCP.
void TCPSender::fill_window() {
  // zero-window probes: If the receiver's window size is zero, the sender
  // should still occasionally send a single byte (if the stream has data) or an
  // empty segment (if the stream is empty) to check if the window has opened
  // up. This is called a "zero-window probe."
  size_t effective_window_size = _window_size > 0 ? _window_size : 1;

  while (true) {
    // Recompute available space each iteration
    // After each segment is sent, _next_seqno advances, so bytes_in_flight()
    // increases.
    // bytes_in_flight() is directly used to compute how much window space is
    // left:

    uint16_t avaiable_window_size = effective_window_size - bytes_in_flight();
    // Your loop condition must allow the loop to run even when the stream is empty, because SYN and FIN don't come from the stream:
    if (avaiable_window_size <= 0)
      break;

    TCPSegment seg;
    // when window has space and something to send, build and send one segment
    // Let me walk through what building one segment looks like at each stage of
    // the connection: Stage 1: _next_seqno == 0          → must send SYN (no
    // payload yet) Stage 2: SYN sent, stream has data → send payload segments
    // Stage 3: stream EOF reached        → attach FIN to last segment

    // Stage 1: _next_seqno == 0          → must send SYN (no payload yet)
    // SYN : Must be the very first thing sent, occupies 1 seq space
    if (!_syn_sent) {
      seg.header().syn = true;
      _syn_sent = true;
      avaiable_window_size -= 1; // SYN occupies one sequence space
    }

    // Stage 2: SYN sent, stream has data → send payload segments
    // Payload segments: each payload byte occupies 1 seq space
    if (_syn_sent && _stream.buffer_size() > 0) {
      // Use Initializer List Form (C++11)— all values must be the same type.
      size_t payload_size =
          min({static_cast<size_t>(avaiable_window_size), _stream.buffer_size(),
               TCPConfig::MAX_PAYLOAD_SIZE});
      seg.payload() = Buffer(_stream.read(payload_size));
      avaiable_window_size -= payload_size; // payload occupies sequence space
    }

    // Stage 3: stream EOF reached        → attach FIN to last segment
    // FIN: Must be the last thing sent, occupies 1 seq space
    if (!_fin_sent && _stream.eof() && avaiable_window_size > 0) {
      seg.header().fin = true;
      _fin_sent = true;
      avaiable_window_size -= 1; // FIN occupies one sequence space
    }

    // --- Guard: nothing to send ---
    // If segment occupies zero sequence space, stop looping
    // This prevents infinite loop when stream is empty and SYN/FIN
    // already sent
    if (seg.length_in_sequence_space() == 0) {
      break;
    }
    // build segment header
    // --- Assign sequence number ---
    // The TCP segment sequence number means: "the byte position of the first
    // payload byte in this segment"
    seg.header().seqno = wrap(_next_seqno, _isn);

    // advance next_seqno by the number of sequence space this segment occupies
    // never by payload.size() alone
    _next_seqno += seg.length_in_sequence_space();

    // Transmit the segment: enqueue the segment for transmission, and update
    // internal state
    _segments_out.push(seg);
    _outstanding_segments.push(
        seg); // keep copy of the sent segment for potential retransmission

    // Start timer when first segment sent
    // If the timer is not already running, start it. If it is already running,
    // leave it alone (don't restart it). The timer tracks the oldest
    // outstanding segment. If it's already running for a previous segment,
    // don't reset it — you'd be giving that old segment more time than it
    // deserves. if(!_timer_running){
    //   _timer_running = true;
    //   _time_elapsed = 0; // reset timer elapsed time
    // }
    if (!_retransmission_timer.running()) {
      _retransmission_timer.start();
    }

    // Stop condition: loop until window is full or no more bytes to send
    if (_stream.buffer_empty() && _fin_sent) {
      break;
    }
  }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size

// TCPConnection calls it when a segment arrives from the remote peer that
// contains a valid ACK. Remote peer sends segment
//         │
//         ▼
// TCPConnection::segment_received(seg)
//         │
//         ├── if seg.header().ack == true:
//         │       _sender.ack_received(seg.header().ackno,
//         │                            seg.header().win)
//         │
//         └── _receiver.segment_received(seg)

// 1. remove fully acked segments from front of queue :
// 2. update window:
// 3. reset timer:  reset timer when something acked
// 4. fill_window()  ← try to send more:
// The TCPSender should fill the window again if new space has opened up.
void TCPSender::ack_received(const WrappingInt32 ackno,
                             const uint16_t window_size) {
  uint64_t ackno_absolute = unwrap(ackno, _isn, _next_seqno);
  if (ackno_absolute > _next_seqno || ackno_absolute < _last_acked_seqno) {
    // ACK number acknowledges data not yet sent, ignore
    // ACK number acknowledges data already acknowledged, ignore
    return;
  }
  // 1. remove fully acked segments from front of queue :
  // The TCPSender should look through its collection of outstanding segments
  // and remove any that have now been fully acknowledged (the ackno is
  // greater than all of the sequence numbers in the segment).

  bool acked_something = false;
  while (!_outstanding_segments.empty()) {
    TCPSegment &seg = _outstanding_segments.front();
    uint64_t seg_seqno_absolute = unwrap(seg.header().seqno, _isn, _next_seqno);
    uint64_t seg_end_seqno_absolute =
        seg_seqno_absolute + seg.length_in_sequence_space();
    if (ackno_absolute >= seg_end_seqno_absolute) {
      // fully acked, remove from queue
      _outstanding_segments.pop();
      acked_something = true;
      _last_acked_seqno = seg_end_seqno_absolute; // update last acked

    } else {
      break;
    }
  }

  // 2. update window:
  _window_size = window_size;

  // 3. reset timer:  reset timer when new data acked
  if (acked_something) {
    _retransmission_timer.reset_rto(); // reset RTO to initial value
    _consecutive_retransmissions = 0;  // reset retransmissions count
    if (_outstanding_segments.empty()) {
      // If there is nothing outstanding, there is nothing to retransmit.
      _retransmission_timer.stop(); // no outstanding segments, stop timer
    } else {
      _retransmission_timer.reset(); // reset timer elapsed time
    }
  }

  // 4. fill_window()  ← try to send more:
  // The TCPSender should fill the window again if new space has opened up.
  fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last
//! call to this method

//  timer not expired → just accumulate ms
//  timer expired     → retransmit front of queue
//                                  if window>0: backoff, increment
//                                  retransmissions reset timer elapsed
void TCPSender::tick(const size_t ms_since_last_tick) {

  _retransmission_timer.tick(
      ms_since_last_tick); // _time_elapsed += ms_since_last_tick;

  // _timer_running && _time_elapsed >= _initial_retransmission_timeout
  if (_retransmission_timer.expired()) {
    // Retransmit the oldest outstanding segment
    if (!_outstanding_segments.empty()) {
      TCPSegment seg = _outstanding_segments.front();
      _segments_out.push(seg); // Enqueue for retransmission

      // If the window size is greater than 0, perform backoff and increment
      // retransmissions Why Only Increment When window_size > 0? When the
      // receiver's window is zero, the sender is doing a zero-window probe —
      // sending a single byte just to check if the window reopened. This is
      // expected behavior, not a sign of network trouble. So:
      if (_window_size > 0) {
        // exponential Backoff: double the retransmission timeout
        _retransmission_timer.double_rto(); // _rto *= 2;
        // Increment the count of consecutive retransmissions
        // It counts how many times in a row the sender had to retransmit
        // because no ACK came back in time.
        _consecutive_retransmissions += 1;
      }
    }
    // Reset timer elapsed, reset elapsed, keep running
    _retransmission_timer.reset(); // _time_elapsed = 0;
  }
}

// TCPConnection (Lab 4) reads this value to decide:
// "Has this connection been retransmitting for too long with no response? If
// so, the remote peer is probably dead — abort the connection." inside
// TCPConnection (Lab 4) if (_sender.consecutive_retransmissions() >
// TCPConfig::MAX_RETX_ATTEMPTS) {
//     // give up — send RST, close connection
// }
unsigned int TCPSender::consecutive_retransmissions() const {
  return _consecutive_retransmissions;
}

// Sometimes TCPConnection needs to send a segment that carries no data and
// occupies no sequence space — purely to transmit an ACK back to the remote
// peer. Receiver sends data to you → your TCPReceiver processes it →
// TCPConnection needs to ACK it → but TCPSender has nothing to send right now
// → solution: send_empty_segment() generates a bare ACK carrier
// Normal segment:    [seqno][SYN?][payload][FIN?]   occupies sequence space
// Empty segment:     [seqno][no SYN][no payload][no FIN]   occupies ZERO
// sequence space
void TCPSender::send_empty_segment() {
  TCPSegment seg;
  // seqno should be next_seqno
  // no SYN, no FIN, no payload
  // length_in_sequence_space() == 0
  seg.header().seqno = wrap(_next_seqno, _isn);

  // NOT pushed to _outstanding_segments
  // NOT starting timer
  // NOT advancing _next_seqno
  _segments_out.push(seg);
}
