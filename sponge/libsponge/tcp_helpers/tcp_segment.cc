#include "tcp_segment.hh"

#include "parser.hh"
#include "util.hh"

#include <variant>

using namespace std;

// This class converts between: Raw bytes  <-->  TCPSegment object
//! \param[in] buffer string/Buffer to be parsed, buffer = received TCP header + payload
//! \param[in] datagram_layer_checksum pseudo-checksum from the lower-layer protocol

ParseResult TCPSegment::parse(const Buffer buffer,
                              const uint32_t datagram_layer_checksum) {
  // TCP checksum is computed over: pseudo-header + TCP header + payload
  // The datagram_layer_checksum is the pseudo-header checksum contribution from IP layer.

  InternetChecksum check(datagram_layer_checksum);
  check.add(buffer);
  // when you recompute checksum over the entire received packet including its checksum field, the result becomes zero
  //In Internet checksum: valid packet => checksum result == 0, otherwise corrupted
  if (check.value()) {
    return ParseResult::BadChecksum;
  }
  // Creates a byte parser over the raw TCP bytes. Parser consumes bytes sequentially.
  NetParser p{buffer};
  // Parse TCP header:  reads fields from the parser
  // After this,  parser cursor moves past TCP header
  _header.parse(p);
  // After header parsing: Remaining bytes = payload
  _payload = p.buffer();
  return p.get_error();
}

// How many TCP sequence numbers this segment consumes
size_t TCPSegment::length_in_sequence_space() const {
  return payload().str().size() + (header().syn ? 1 : 0) +
         (header().fin ? 1 : 0);
}

// Converts TCPSegment object → raw network bytes
//! \param[in] datagram_layer_checksum pseudo-checksum from the lower-layer
//! protocol
BufferList TCPSegment::serialize(const uint32_t datagram_layer_checksum) const {
  TCPHeader header_out = _header;
  // Zero checksum field: When computing checksum, checksum field itself must be zero. 
  header_out.cksum = 0;
  // calculate checksum -- taken over entire segment
  InternetChecksum check(datagram_layer_checksum);
  check.add(header_out.serialize());
  check.add(_payload);
  // nternet checksum stores: 1's complement of the sum，Meaning: checksum = ~sum
  header_out.cksum = check.value();

  BufferList ret;
  // This enables:zero-copy networking
  ret.append(header_out.serialize());
  ret.append(_payload);

  return ret;
}
