#include "tcp_receiver.hh"
#include "wrapping_integers.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

// The Big Idea: Given this incoming TCP segment, what bytes can now be safely
// delivered to the application, and what ACK should we send back? TCP packet
// world <---> ordered byte-stream world What segment_received() Must Do:
// 1. Validate segment state
// 2. Process SYN
// 3. Convert seqno → stream index
// 4. Push payload into reassembler
// 5. Handle FIN
// 6. Update ACK state

// | Concept         | Meaning                                         |  Data type    |
// | --------------- | ----------------------------------------------- |---------------|
// | TCP seqno       | byte position in TCP sequence space             | WrappingInt32 |
// | absolute stream positions| always starts at zero and doesn’t wrap | uint64_t     |
// | stream index    | byte position in logical byte stream            | uint64_t     |
// | ACK             | next seqno receiver wants                       |               |
// | bytes_written() | contiguous stream bytes assembled               |               |


void TCPReceiver::segment_received(const TCPSegment &seg) {

  if (!_syn_received && !seg.header().syn) {
    // 1. Ignore everything before SYN, TCP connection starts only after SYN.
    return;
  } else if (!_syn_received && seg.header().syn) {
    // 2. Record Initial Sequence Number (ISN)
    // Only the SYN segment has syn == true. All subsequent segments have syn ==
    // false.
    _syn_received = true;
    // SYN segement seqno IS the sender's ISN
    // Sequence numbers are meaningless without synchronization.
    _isn = seg.header().seqno;
  }

  bool eof = false;
  if (seg.header().fin) {
    eof = true;
  }

  // 3. Convert TCP seqno → stream index
  // mapping between TCP sequence space -> StreamReassembler byte index space
  // Stream Index Space: Counts ONLY payload bytes,Starts at 0
  // Absolute TCP Sequence Space : Counts SYN , payload bytes, FIN,Starts at 0
  // each TCP segment header contains only one sequence number field.
  // The TCP segment sequence number means: "the byte position of the first
  // payload byte in this segment" TCP stream index starts at 0,  But TCP
  // sequence numbering starts at:ISN + 1,  but SYN consumes one seqno, for
  // payload bytes: each payload byte consumes one seqno, Sequence Numbers Refer
  // To Bytes stream_index = seqno - ISN - 1

  // Real TCP header contains:   TCP sequence numbers are 32-bit absolute
  // wrapping seqno CS144 Uses WrappingInt32 to represent wrapping sequence
  // numbers. In Real TCP Header stores uint32_t
  // WrappingInt32 sqeno -> unwrap ->  uint64_t absolute seqno -> stream index
  
  
  // Big Idea : TCP packets almost always arrive near
  //   checkpoint : use the index of the last reassembled byte 
  //  _reassembler.stream_out().bytes_written() : number of contiguous stream payload  bytes assembled
  uint64_t assembled_size = _reassembler.stream_out().bytes_written();
  // +1 accounts for SYN consuming sequence space, so it meand next expected absolute sequence number instead of first_unassembled stream index
  uint64_t next_expected_absolute_seqno = assembled_size +1;

  uint64_t absolute_sqeno = unwrap(seg.header().seqno, _isn, next_expected_absolute_seqno);

    // | Case           | absolute_seqno | syn | payload_absolute | stream_index |
    // | -------------- | -------------- | --- | ---------------- | ------------ |
    // | SYN only       | 0              | 1   | 1                | 0            |
    // | SYN+"abc"      | 0              | 1   | 1                | 0            |
    // | normal payload | 5              | 0   | 5                | 4            |
    // | None SYN+ empty payload|
  uint64_t payload_absolute_seqno = absolute_sqeno + seg.header().syn;
  uint64_t stream_index =  payload_absolute_seqno -1 ; 


  // 4. Push Payload into Reassembler
  // Receiver Infers Remaining Byte Positions, Payload length tells receiver how
  // many bytes follow. one seqno + payload length defines seqnos for all bytes
  _reassembler.push_substring( seg.payload().copy(), stream_index, eof);

  // 6. Compute ACK Number
  // ACK means: first unassembled stream index, next byte receiver wants, NOT last byte received
  // ACK absolute sequence number is: 1 for SYN + bytes_written + 1 if FIN assembled
  // Handle FIN:  FIN consumes one seqn, FIN is not data,But it still advances sequence numbers.
  // Suppose stream ended,Then ACK must advance one more.
  assembled_size = _reassembler.stream_out().bytes_written(); //ecompute bytes_written
  uint64_t ack_absolute = 1 + assembled_size + _reassembler.stream_out().input_ended();
  _ackno = wrap(ack_absolute, _isn);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
  if (_syn_received) {
    // SYN received → we have a valid ACK number
    return _ackno;
  }
  // std::nullopt is a special constant that represents "this optional has no value" — it's how you construct an empty std::optional
  // no SYN yet → no meaningful ACK to send
  return nullopt;
}

size_t TCPReceiver::window_size() const {
    return _capacity -  _reassembler.stream_out().buffer_size(); }
