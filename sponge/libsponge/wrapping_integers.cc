#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.



using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a
//! WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
  // Take absolute TCP position and project it onto the 32-bit TCP clock.
  // TCP seqno only stores low 32 bits of the absolute 64-bit sequence number
  // seqno  =  (absolute no + isn) mod 2^32
  // static_cast<uint32_t>(n):  
  // truncate to lower 32 bits, which automatically gives modulo 2^32 behavior(n mod 2^32).
  // isn.raw_value() + static_cast<uint32_t>(n): 
  // unsigned integer overflow in C++ naturally implements modulo arithmetic,uint32_t overflow already wraps naturally in C++,
  
  WrappingInt32 seqno = WrappingInt32(static_cast<uint32_t>(n) + isn.raw_value() );

  return seqno;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number
//! (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to
//! `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One
//! stream runs from the local TCPSender to the remote TCPReceiver and has one
//! ISN, and the other stream runs from the remote TCPSender to the local
//! TCPReceiver and has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
  // Checkpoint is: “Where do we currently believe we are in the stream?”
  // Checkpoint works because: TCP streams progress continuously forward, so the correct absolute seqno should be near current receiver state.
  // TCP assumes: packets arriving now are probably near current stream position.
  // chooses: the absolute seqno whose wrapped value matches n AND is closest to checkpoint.
  
  uint64_t TWO32 = 1ull << 32;

  // 1. Compute Relative Offset = absolute seqno modulo 2^32
  uint32_t offset = n.raw_value() - isn.raw_value();

  // 2. Find Nearby Cycle
  // candidate = (base cycle near checkpoint) + offset = which cycle*2^32 + offset 
  uint64_t candidate =  (checkpoint / TWO32) * TWO32 + offset;

  // The nearest candidate might actually be: one cycle above or one cycle below
  // because offset may shift us across boundary.so must compare:
  // candidate / candidate + 2^32 / candidate - 2^32, and choose closest.
  
  // With unsigned integers: subtraction is MUCH more dangerous than addition.
  if(candidate +  TWO32/2 < checkpoint){
    // if candidate is more than half-cycle BELOW checkpoint
    // which means candidate move up one cycle will closer to checkpoint
    candidate  += TWO32;
  }else if(candidate > checkpoint+ TWO32/2 && candidate >= TWO32){
    // if candidate is more than half-cycle ABOVE checkpoint
    // candidate - TWO32/2 > checkpoint:  can subtraction underflow in uint64_t arithmetic,
    // candidate >= TWO32, prevents illegal wrap.
    candidate -= TWO32;
  }
  return  candidate;
}
