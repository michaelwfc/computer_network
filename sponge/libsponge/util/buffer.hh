#ifndef SPONGE_LIBSPONGE_BUFFER_HH
#define SPONGE_LIBSPONGE_BUFFER_HH

#include <algorithm>
#include <deque>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <sys/uio.h>
#include <vector>

//! \brief A reference-counted read-only string that can discard bytes from the front
// a lightweight, zero-copy, shared immutable byte slice
// The core idea is: multiple Buffer objects can share the same underlying string without copying memory
class Buffer {
private:
  // shared ownership of actual byte storage. 
  // Multiple Buffers can point to same string. The underlying string storage. 
  // Multiple Buffer objects can share the same
  std::shared_ptr<std::string> _storage{};

  // where this Buffer's visible region begins. 
  // Multiple Buffer objects can share the same underlying string, but they can have different starting offsets and sizes.
  size_t _starting_offset{}; 


public:
  Buffer() = default;

  //! \brief Construct by taking ownership of a string
  // This takes ownership of a string.

  // std::string &&str :  
  // && indicates an rvalue reference, which is a reference that can bind to a temporary object (an rvalue).
  // an rvalue reference to a std::string, Used for move semantics.  usually temporary object.
  // which allows the constructor to take ownership of the string without copying it. 


  // noexcept: 
  // Move operations often marked noexcept because:  moving should not fail +  STL containers optimize better if move noexcept

  // std::make_shared<std::string>: 
  // Receive (temporary) string., Move string object on heap(NOT copy),  and returns a shared_ptr<string> owning it.
  // memory automatically freed +  reference counted +  safe sharing
  // make_shared is a function template, not a complete object.You must:  specify the type + provide constructor arguments
  // make_shared<T>(args...): using std::string constructor with argument std::move(str) 
  // creates an object of type T, forwarding args to T's constructor, and returns a shared_ptr<T> that owns the object.

  // std::move(str) :  
  // move semantics are one of the most important ideas in modern C++.
  // It allows you to transfer ownership of resources from one object to another without copying, which can lead to significant performance improvements.
  // Move Into Heap String， moves internal memory instead of copying.
  // used to indicate that the string can be moved rather than copied, enabling efficient transfer of resources.

  // default initialization for _starting_offset， _starting_offset = 0 automatically 
  Buffer(std::string &&str) noexcept
      : _storage(std::make_shared<std::string>(std::move(str))) {}

  //! \name Expose contents as a std::string_view
  //!@{
  // returns: non-owning view into current visible bytes. string_view does NOT own memory.
  // What is string_view? Think: pointer + length 
  // std::string = owns the text memory
  // std::string_view = just points at existing text memory without copying
  std::string_view str() const {
    if (not _storage) {
      return {};
    }
    return {_storage->data() + _starting_offset,
            _storage->size() - _starting_offset};
  }

  // Conversion Operator
  // “An object of this class can be automatically converted into std::string_view.”
  // “A Buffer can automatically behave like a std::string_view.”
  // This allows automatic conversion: 
  // Without conversion operator:
  // Buffer b(...);
  // std::string_view sv = b.str();
  // With conversion operator: 
  // Buffer b(...);
  // std::string_view sv = b; 
  // Automatic conversion happens , is transformed by the compiler into: 
  // std::string_view sv = b.operator std::string_view();  which returns: b.str()
  // Buffer OWNS memory, std::shared_ptr<std::string> _storage; This owns heap memory. string_view DOES NOT own memory
  // Buffer b(std::string("hello"));
  //   Buffer b
  // └── shared_ptr
  //       └── heap string: "hello"

  // std::string_view sv = b;  does NOT transfer ownership, it only creates:(pointer + length), pointing into Buffer's memory.
  operator std::string_view() const { return str(); }
  //!@}

  //! \brief Get character at location `n`
  // Reads byte from visible region.
  uint8_t at(const size_t n) const { return str().at(n); }

  //! \brief Size of the string
  // returns visible size only, NOT full underlying string size.
  // str() -> This returns a string_view. And the visible part begins at: _storage->data() + _starting_offset
  size_t size() const { return str().size(); }

  //! \brief Make a copy to a new std::string
  // Actually creates new owned string.
  std::string copy() const { return std::string(str()); }

  //! \brief Discard the first `n` bytes of the string (does not require a copy
  //! or move)
  //! \note Doesn't free any memory until the whole string has been discarded in
  //! all copies of the Buffer.
  // Instead of: physically deleting bytes (which requires memory movement)
  // it simply:  _starting_offset += n;
  void remove_prefix(const size_t n);
};

//! \brief A reference-counted discontiguous string that can discard bytes from
//! the front
//! \note Used to model packets that contain multiple sets of headers
//! + a payload. This allows us to prepend headers (e.g., to
//! encapsulate a TCP payload in a TCPSegment, and then encapsulate
//! the TCPSegment in an IPv4Datagram) without copying the payload.

// Why do we need BufferList?  This is called: scatter/gather I/O
// Suppose TCP segment: [TCP header][payload],Instead of copying them together into one big string: std::string combined = header + payload;which allocates + copies memory,
// we can store:  Buffer(serialized_header) , Buffer(payload) inside:BufferList 
// TCPHeader object
//     ↓ serialize()
// std::string of raw TCP bytes
//     ↓ Buffer(...)
// Buffer
//     ↓ append()
// BufferList



class BufferList {
private:
  std::deque<Buffer> _buffers{};

public:
  //! \name Constructors
  //!@{

  BufferList() = default;

  //! \brief Construct from a Buffer
  // creates _buffers = [buffer]
  BufferList(Buffer buffer) : _buffers{buffer} {}

  //! \brief Construct by taking ownership of a std::string
  // string -> Buffer -> BufferList,  without copying string contents.
  BufferList(std::string &&str) noexcept {
    Buffer buf{std::move(str)};
    append(buf);
  }
  //!@}

  //! \brief Access the underlying queue of Buffers
  const std::deque<Buffer> &buffers() const { return _buffers; }

  //! \brief Append a BufferList
  void append(const BufferList &other);

  //! \brief Transform to a Buffer
  //! \note Throws an exception unless BufferList is contiguous
  operator Buffer() const;

  //! \brief Discard the first `n` bytes of the string (does not require a copy
  //! or move)
  void remove_prefix(size_t n);

  //! \brief Size of the string
  size_t size() const;

  //! \brief Make a copy to a new std::string
  std::string concatenate() const;
};

//! \brief A non-owning temporary view (similar to std::string_view) of a
//! discontiguous string
class BufferViewList {
  std::deque<std::string_view> _views{};

public:
  //! \name Constructors
  //!@{

  //! \brief Construct from a std::string
  BufferViewList(const std::string &str)
      : BufferViewList(std::string_view(str)) {}

  //! \brief Construct from a C string (must be NULL-terminated)
  BufferViewList(const char *s) : BufferViewList(std::string_view(s)) {}

  //! \brief Construct from a BufferList
  BufferViewList(const BufferList &buffers);

  //! \brief Construct from a std::string_view
  BufferViewList(std::string_view str) {
    _views.push_back({const_cast<char *>(str.data()), str.size()});
  }
  //!@}

  //! \brief Discard the first `n` bytes of the string (does not require a copy
  //! or move)
  void remove_prefix(size_t n);

  //! \brief Size of the string
  size_t size() const;

  //! \brief Convert to a vector of `iovec` structures
  //! \note used for system calls that write discontiguous buffers,
  //! e.g. [writev(2)](\ref man2::writev) and [sendmsg(2)](\ref man2::sendmsg)
  // This directly prepares data for OS scatter/gather syscalls.
  std::vector<iovec> as_iovecs() const;
};

#endif // SPONGE_LIBSPONGE_BUFFER_HH
