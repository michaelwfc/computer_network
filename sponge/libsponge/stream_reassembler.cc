#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in
// `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _first_unassembled_index(0),
      _buffer(), _eof_received(false), _eof_index(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
// "These bytes belong starting at stream byte position index."
// @param: index: 0-based byte-stream index
// StreamReassembler Layer Responsible for ONLY: stream assembly
// pure byte-stream index space, NOT TCP sequence-number space.
// It knows NOTHING about: TCP,seqno, SYN,wapping, ACK
void StreamReassembler::push_substring(const string &data, const size_t index,
                                       const bool eof) {
  // 0. if eof, remember the EOF position
  if (eof) {
    _eof_received = true;
    _eof_index = index + data.size(); // one-past-the-end position
  }

  // 1. compute the acceptable window: "I can only accept bytes that fit in
  // memory." Determine which incoming bytes fit inside the receiver’s currently
  // available memory window.
  // incoming substring: [index, index + data.size())]
  // acceptable window: [_first_unassembled_index, _first_unassembled_index +
  // remaining_capacity)]

  // This capacity limits both the bytes that have been reassembled, and those  that have not yet been reassembled. 
  // _output.buffer_size(): the bytes that  have been reassembled but not yet consumed by the reader.
  //  _buffer.size() : the bytes have not yet been reassembled but buffered.
  size_t assembled_bytes_size = _output.buffer_size();
  size_t unassembled_bytes_size = _buffer.size();
  size_t remaining_capacity = _capacity - assembled_bytes_size - unassembled_bytes_size ;
  
  size_t acceptable_window_start = max(index, _first_unassembled_index);
  // Fix: capacity window is relative to first_unassembled, 
  // limited only by what ByteStream can hold (its own buffer_size)
  size_t first_unacceptable_index = _first_unassembled_index + remaining_capacity+ unassembled_bytes_size;
  size_t acceptable_window_end =   min(index + data.size(), first_unacceptable_index);
  if (acceptable_window_start < acceptable_window_end) {

    size_t offset = acceptable_window_start - index;
    size_t length = acceptable_window_end - acceptable_window_start;
    // extract the substring contains Only acceptable bytes
    string substring = data.substr(offset, length);

    // 2. unified-buffer design:
    // all unassembled bytes live in _buffer,then: drain contiguous prefix into
    // ByteStream 2.1. Insert ALL valid bytes into buffer Byte-Level Loop
    // Processing
    for (size_t i = 0; i < substring.size(); i++) {
      size_t byte_index = acceptable_window_start + i;
      if (_buffer.count(byte_index)) {
        // already-buffered
        continue;
      }
      if (byte_index < _first_unassembled_index) {
        // already assembled
        continue;
      } else {

        // NOT directly write incoming substring. Just normalize incoming bytes
        // into buffer.
        //  With unified-buffer design, ALL unassembled bytes live in
        //  buffer,Then assembly = consume contiguous prefix from buffer
        _buffer[byte_index] = substring[i];
      }
    }

    // 2.2. Assemble contiguous bytes from buffer:
    // Single unified drain loop AFTER insertion.
    std::string assembled;
    // drain contiguous bytes from buffer
    while (_buffer.count(_first_unassembled_index)) {
      // consume from buffer
      char c = _buffer[_first_unassembled_index];
      assembled += c;
      _buffer.erase(_first_unassembled_index);
      _first_unassembled_index++;
    }
    if (assembled.size() > 0) {
      _output.write(assembled);
    }
  }

  // 3. Handle EOF
  // Then EOF logic below still executes even if incoming data is not
  // acceptable, because EOF may arrive before all data. if EOF seen AND all
  // bytes assembled: _output.end_input(), Now stream officially closed, no
  // future bytes will EVER arrive
  if (_eof_received && _first_unassembled_index == _eof_index) {
    _output.end_input();
  }
}

size_t StreamReassembler::unassembled_bytes() const { return _buffer.size(); };

bool StreamReassembler::empty() const { return _buffer.empty(); }
