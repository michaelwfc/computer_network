#include "byte_stream.hh"
#include <cstddef>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in
// `byte_stream.hh`

/**
Template allows you to write generic code that works with different types:
- typename... Targs - variadic template (accepts any number of type arguments)
- Targs &&... - variadic parameter pack (accepts any number of arguments)
- This DUMMY_CODE function is a placeholder that does nothing - it just accepts any arguments and ignores them
- You'll remove all DUMMY_CODE calls when implementing real functionality

*/
template <typename... Targs> void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

/**
use member initialization lists

*/
ByteStream::ByteStream(const size_t capacity)
   : _capacity(capacity){} // Only initialize what's not default
  // _capacity = capacity;// Assignment (not initialization)  

/**
push_back(char): Add a character to the back
push_front(char): Add a character to the front

pop_back(): Remove last character
pop_front(): Remove first character

Indexing: 
_buffer[0] = front of queue (oldest element)
_buffer[1] = second element
...
_buffer[_buffer.size()-1] = back of queue (newest element)
*/
size_t ByteStream::write(const string &data) {
  // Loop through each character, write the data to the back of deque
  size_t writable = std::min(data.size(), remaining_capacity());
  char c;
  for(size_t i=0; i < writable; i++){
    c= data[i];
    _buffer.push_back(c);
  }
  _bytes_written += writable; // Update statistics
  return writable;
}

//! \param[in] len bytes will be copied from the output side of the buffer
// "Peek" means look but don't remove
string ByteStream::peek_output(const size_t len) const {
  std::string result;
  size_t count = std::min(len, _buffer.size());
  for(size_t i = 0; i < count; i++){
    result += _buffer[i]; // Access without removing
  }
  return result;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
  size_t count = std::min(len, _buffer.size());
  for(size_t i = 0; i < count; i++){
    _buffer.pop_front(); // Remove from front
  }

  _bytes_read += count; 
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
  std::string result= peek_output(len);
  pop_output(result.size());  
  return result;
}

// called by the writer to Signal that the byte stream has reached its ending
//Without end_input(): The reader would wait forever, not knowing if more data is coming!
void ByteStream::end_input() {
  _input_ended = true;

}

bool ByteStream::input_ended() const { 
  return _input_ended;
 }

size_t ByteStream::buffer_size() const { 
  return _buffer.size();
 }

bool ByteStream::buffer_empty() const { 
  return _buffer.empty();
 }


/**
`true` if the output has reached the ending
To know if the output has reached the ending, you need to track whether the input has been ended and whether the buffer is empty. The eof() method should return true when both conditions are met:

- Input has ended: The end_input() method has been called (meaning no more data will be written)
- Buffer is empty: All remaining data in the buffer has been read out

 */
bool ByteStream::eof() const { 
  return input_ended() && buffer_empty();
}

// Total number of bytes written
size_t ByteStream::bytes_written() const { 
  return _bytes_written;
 }

size_t ByteStream::bytes_read() const { 
  return _bytes_read;
 }

size_t ByteStream::remaining_capacity() const { 
  return _capacity - _buffer.size();
 }
