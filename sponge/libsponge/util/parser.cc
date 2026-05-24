#include "parser.hh"

using namespace std;

//! \param[in] r is the ParseResult to show
//! \returns a string representation of the ParseResult
string as_string(const ParseResult r) {
  static constexpr const char *_names[] = {
      "NoError",        "BadChecksum",    "PacketTooShort",
      "WrongIPVersion", "HeaderTooShort", "TruncatedPacket",
  };

  return _names[static_cast<size_t>(r)];
}

void NetParser::_check_size(const size_t size) {
  if (size > _buffer.size()) {
    set_error(ParseResult::PacketTooShort);
  }
}

// Create a generic function for integer type T, which can be u8, u16, or u32.
// _parse_int<uint8_t>()
// _parse_int<uint16_t>()
// _parse_int<uint32_t>()
// Parser converts: byte stream（raw bytes of a network packet） -> an integer type (u8, u16, u32)  -> structured fields
// using network byte order (big-endian).
template <typename T> T NetParser::_parse_int() {
  // constexpr means compile-time constant.
  constexpr size_t len = sizeof(T);
  _check_size(len);
  if (error()) {
    return 0;
  }

  // Parse bytes one-by-one
  T ret = 0;
  for (size_t i = 0; i < len; i++) {
    // Why shift left by 8? Because network byte order is Big-endian（Most significant byte first）
    // Example: 
    // Suppose a sender wants to send:  _buffer = [0x12, 0x34] 
    // This is two bytes: 0x12 and 0x34. Networking protocol says: This means integer 0x1234
    // In memory, this integer is stored as bytes.  But Different CPUs store bytes differently internally.
    // Big-endian is stored as 0x12 0x34.  store the most significant byte at the smallest memory address. 
    // Little-endian：Stores   0x34 0x12 ， Least Significant Byte first
    // So we shift the existing value left by 8 bits,move previous bytes one byte upward, make room for the next byte.
    ret <<= 8;
    ret += uint8_t(_buffer.at(i));
  }
  // Consume bytes from buffer, advances parser cursor.
  // Return parsed integer
  _buffer.remove_prefix(len);

  return ret;
}

void NetParser::remove_prefix(const size_t n) {
  _check_size(n);
  if (error()) {
    return;
  }
  _buffer.remove_prefix(n);
}

// Constructing the binary wire-format packet.
// integers → network-order bytes as Big-endian（Most significant byte first）
// The resulting bytes are exactly what gets transmitted over Ethernet/TCP/IP.
// That is the bridge between: structured program data and raw network bytes on the wire
// Example:
// Suppose we want to send integer 0x1234, which is two bytes: 0x12 and 0x34.
// We want to convert this integer into a byte stream that can be sent over the network.
// Final serialized packet bytes Resulting string contains: [0x12 0x34], which is Big-endian / network byte order
// 
template <typename T> void NetUnparser::_unparse_int(string &s, T val) {
  constexpr size_t len = sizeof(T);
  for (size_t i = 0; i < len; ++i) {
    // This extracts bytes from most-significant to least-significant.
    const uint8_t the_byte = (val >> ((len - i - 1) * 8)) & 0xff;
    // In modern C++: std::string is basically: a dynamic array of bytes/chars,  Conceptually similar to: std::vector<char>
    // std::string stores bytes sequentially in insertion order.
    // s.push_back(the_byte) means: Append one character (one byte) to the end of the string.
    // Here  In networking code , the string is NOT really “text”. It is really just contiguous bytes ,acting as: A binary byte buffer.
    s.push_back(the_byte);
  }
}

uint32_t NetParser::u32() { return _parse_int<uint32_t>(); }

uint16_t NetParser::u16() { return _parse_int<uint16_t>(); }

uint8_t NetParser::u8() { return _parse_int<uint8_t>(); }

void NetUnparser::u32(string &s, const uint32_t val) {
  return _unparse_int<uint32_t>(s, val);
}

void NetUnparser::u16(string &s, const uint16_t val) {
  return _unparse_int<uint16_t>(s, val);
}

void NetUnparser::u8(string &s, const uint8_t val) {
  return _unparse_int<uint8_t>(s, val);
}
