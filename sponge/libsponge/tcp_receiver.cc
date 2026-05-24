#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

//
void TCPReceiver::segment_received(const TCPSegment &seg) { 
    // keep track of the ISN of the first SYN segment we receive
    if (seg.header().syn && !_isn.has_value()) {
        _isn = seg.header().seqno;
    }
 }

optional<WrappingInt32> TCPReceiver::ackno() const { return {}; }

size_t TCPReceiver::window_size() const { return {}; }
