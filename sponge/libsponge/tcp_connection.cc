#include "tcp_connection.hh"
#include "tcp_segment.hh"

#include <iostream>
#include <limits>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

using namespace std;

// Initiates a TCP handshake. Internally it tells the sender to send its very
// first segment — a SYN. This is the active open: Client Server
//   │──── SYN seq=ISN ──────────►  │
//   │◄─── SYN+ACK seq=ISN' ──────  │
//   │──── ACK ──────────────────►  │
// connect() is only called on the initiating side.
// The other side (passive open / server) begins instead when it receives its
// first SYN via segment_received().
// connect() called
//   _sender.fill_window()
//     _syn_sent = false → send SYN
//     _sender.segments_out = [SYN]

// push_segments_out():
//   iteration 1:
//     seg = SYN
//     _receiver.ackno().has_value() == false   ← no SYN received yet
//       → ack = false, no ackno set
//     win = receiver.window_size()
//     push to _segments_out

//   loop ends (_sender.segments_out empty)

void TCPConnection::connect() {
  // When fill_window() is called and the sender has never sent anything (next
  // seqno = 0), it produces a SYN segment automatically. SYN pushed to
  // _segments_out
  _sender.fill_window();

  push_segments_out();
}

void TCPConnection::segment_received(const TCPSegment &seg) {
  // Unclean shotdown
  // RST received -> kill both stream immediately
  if (seg.header().rst) {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
    return;
  }

  // reset linger time
  _time_since_last_segment_received = 0;


  // Normal processing
  // 1. Pass the segment to the receiver, which will process it and update its
  // ACK state.
  _receiver.segment_received(seg);

  // 2. If the incoming segment has a valid ACK, pass the ACK information to the
  // sender, which will update its state and possibly send new segments.
  if (seg.header().ack) {
    _sender.ack_received(seg.header().ackno, seg.header().win);
  }

  // ── LINGER FLAG UPDATE ────────────────────────────────────────
  // If inbound stream just ended AND we have not sent FIN yet
  // → remote closed first → passive close → no linger needed
  // _receiver.stream_out().input_ended()： 
  // The inbound stream has been fully reassembled AND the FIN has been processed.
  //In other words: remote peer sent FIN， we received it，reassembler delivered it to the byte stream，  → remote has no more data to send to us
  // !_sender.fin_sent()： 
  // We have NOT yet sent our own FIN.
  // In other words: local application has NOT yet called end_input_stream()， OR end_input_stream() was called but FIN not yet transmitted， 
  // → we have not committed to closing our outbound stream yet
  // if (remote's FIN arrived) AND (we haven't sent our FIN yet)
  // → remote closed their outbound stream BEFORE we closed ours
  // → remote was the FIRST to initiate closing
  // → this is PASSIVE CLOSE from our perspective

  if (_receiver.stream_out().input_ended() && !_sender.fin_sent()) {
  
        _linger_after_streams_finish = false;
    }


  // ── MUST REPLY RULE ───────────────────────────────────────────
  // [3] always try to fill window
  // This is what produces the server's SYN on the first call
  _sender.fill_window();

  // 4. if received segment occupied sequence space (e.g. not pure ACK),
  // TCPConnection must ensure at least one segment goes out:
  if (seg.length_in_sequence_space() > 0) {

    if (_sender.segments_out().empty()) {
      // if sender has nothing else to send, it still needs to send an empty
      // segment (bare ACK) back to the peer to update the peer's ACK state.
      // fill_window() had nothing to produce  → manufacture a bare ACK via
      // send_empty_segment() send_empty_segment() used as the reply to SYN
      _sender.send_empty_segment();
    }
    // if sender already has segments queued (e.g. from fill_window),
    // those will carry the ACK anyway via push_segments_out()
  }

  // 5. Drain sender queue -> enrich each segement -> push to _segments_out for
  // transmission
  push_segments_out();
}

// The sender only sets: seqno, SYN, FIN, payload.
// The receiver only computes: ackno, window_size.
// TCPConnection merges them in push_segments_out().
// enriched with ackno/window, moved to _segments_out, sent to network
// Every place that might cause _sender to produce segments must be followed by push_segments_out().
void TCPConnection::push_segments_out() {
  while (!_sender.segments_out().empty()) {
    TCPSegment seg = _sender.segments_out().front();
    _sender.segments_out().pop();

    // enrich the segment with reciver's fields
    // THIS is where ACK flag is set — always here, never in sender
    if (_receiver.ackno().has_value()) {
      seg.header().ack = true;
      seg.header().ackno = _receiver.ackno().value();
    }

    // clamp window size to uint16_t max
    seg.header().win = static_cast<uint16_t>(
        min(_receiver.window_size(),
            static_cast<size_t>(numeric_limits<uint16_t>::max())));

    _segments_out.push(seg);
  }
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to
//! this method
void TCPConnection::tick(const size_t ms_since_last_tick) {

  // advance timers
  _time_since_last_segment_received += ms_since_last_tick;
  
  _sender.tick(ms_since_last_tick);

  // Unclean shutdown
  // too many retransmissions -> remote is dead -> send RST
  if (_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS) {
    send_rst();
    return;
  }

  // _sender.tick() may produce retransmitted segments sitting in _sender.segments_out
  push_segments_out(); 

  // active() reads _time_since_last_segment_received and all prereqs nothing extra needed here — active() handles the logic

}

// The RST Helper: Used by tick() and destructor
void TCPConnection::send_rst(){
  // get a segment with correct seqno
  _sender.send_empty_segment();

  // grab it and set RST flage
  TCPSegment seg = _sender.segments_out().front();
  _sender.segments_out().pop();
  seg.header().rst = true;

  // add ack if receiver has valid ackno
  if(_receiver.ackno().has_value()){
    seg.header().ack=true;
    seg.header().ackno = _receiver.ackno().value();
  }

  _segments_out.push(seg);


  // kill both stream
  _sender.stream_in().set_error();
  _receiver.stream_out().set_error();
  _active = false;



}

// Hands bytes from the application into the outbound ByteStream (which lives
// inside the sender). Returns how many bytes were actually accepted, it may be
// less than requested if the buffer is nearly full. application bytes
//       │
//       ▼
//   outbound ByteStream  (inside TCPSender)
//       │
//       ▼  (TCPSender reads from here, packetizes, produces segments)
//   _segments_out
//       │
//       ▼
//      IP / UDP
size_t TCPConnection::write(const string &data) {

  size_t bytes_written = _sender.stream_in().write(data);
  // After accepting new data from the application, the sender should try to
  // fill the window again (i.e. send more segments if the window allows).
  _sender.fill_window();

  push_segments_out();
  return bytes_written;
}

// The application signals "I am done writing." This writes EOF into the
// outbound ByteStream. The sender will eventually convert this into a FIN
// segment on the wire. This is what happens when a user presses Ctrl-D in a
// terminal.
void TCPConnection::end_input_stream() {
  _sender.stream_in().end_input();
  // After accepting EOF from the application, the sender should try to fill the
  // window again (i.e. send a FIN segment if the window allows).
  _sender.fill_window();

  push_segments_out();
}

// How many bytes can the application currently write without blocking.
// This is just the available space in the outbound ByteStream.
// It is determined by send_capacity minus how many bytes are already buffered
// but not yet sent/acknowledged.
size_t TCPConnection::remaining_outbound_capacity() const {
  return _sender.stream_in().remaining_capacity();
}

size_t TCPConnection::bytes_in_flight() const {
  return _sender.bytes_in_flight();
}

size_t TCPConnection::unassembled_bytes() const {
  return _receiver.unassembled_bytes();
}

size_t TCPConnection::time_since_last_segment_received() const {
  return _time_since_last_segment_received;
}

//  Clean Shutdown Decision
// This is the central termination logic. All four prerequisites checked here:
bool TCPConnection::active() const {
  // Unclean shutdown
  if (!_active)
    return false; // RST sent or received

  // ── CLEAN SHUTDOWN PREREQUISITES ─────────────────────────────
  // Prereq 1: inbound stream fully assembled and ended
  bool prereq1 = _receiver.stream_out().input_ended();

  // Prereq 2: outbound stream fully sent
  bool prereq2 = _sender.fin_sent();

  // Prereq 3: outbound stream fully acknowledged
  bool prereq3 = _sender.bytes_in_flight() == 0;

  // not all prereqs met -> still alive
  if (!prereq1 || !prereq2 || !prereq3) {
    return true;
  }

  // ── PREREQS 1-3 MET: check prereq 4 ─────────────────────────
  // Prereq 4: remote peer KNOWS we received everything
  // Option B: passive close → no linger → done immediately
  if(!_linger_after_streams_finish) return false;

  // Option A: active close → must linger 10 × rt_timeout
  return (_time_since_last_segment_received < 10*_cfg.rt_timeout);
}

TCPConnection::~TCPConnection() {
  try {
    if (active()) {
      cerr << "Warning: Unclean shutdown of TCPConnection\n";

      // Your code here: need to send a RST segment to the peer
      // ── UNCLEAN SHUTDOWN ──────────────────────────────────
      // connection destroyed while still open
      // remote peer must be told via RST
      send_rst();
    }
  } catch (const exception &e) {
    std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
  }
}
