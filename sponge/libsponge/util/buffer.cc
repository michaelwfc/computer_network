#include "buffer.hh"
#include <stdexcept>

using namespace std;




void Buffer::remove_prefix(const size_t n) {
  if (n > str().size()) {
    throw out_of_range("Buffer::remove_prefix");
  }
  _starting_offset += n;
  if (_storage and _starting_offset == _storage->size()) {
    // “We have consumed the ENTIRE underlying string.”
    // This is a shared_ptr operation. means: “This Buffer no longer owns the
    // string.” Equivalent conceptual behavior: _storage = nullptr;
    _storage.reset();
  }
}

// This does NOT deep-copy string contents. Why?
// Because Buffer internally uses: shared_ptr<string>, So copying a Buffer only copies: pointer+ offset, Very cheap.
void BufferList::append(const BufferList &other) {
  // const auto &buf means:
  // auto → let the compiler automatically figure out the type
  // & → use a reference (do not make a copy),Without &:Without & each Buffer would be copied.
  // const → read-only, cannot modify it
  for (const auto &buf : other._buffers) {
    _buffers.push_back(buf);
  }
}

// This is a conversion operator. allows: Buffer b = buffer_list;  BUT ONLY if list contains exactly ONE buffer.
BufferList::operator Buffer() const {
  switch (_buffers.size()) {
  case 0:
    return {};
  case 1:
    return _buffers[0];
  default: {
    throw runtime_error("BufferList: please use concatenate() to combine a "
                        "multi-Buffer BufferList into one Buffer");
  }
  }
}

// This DOES perform copying to create one contiguous string.
string BufferList::concatenate() const {
  std::string ret;
  ret.reserve(size());
  for (const auto &buf : _buffers) {
    // For each Buffer:
    // buf converts to std::string_view:  Buffer has operator string_view()
    // std::string::append() copies the bytes, Not BufferList::append()
    // bytes are appended into ret
    ret.append(buf);
  }
  return ret;
}

size_t BufferList::size() const {
  size_t ret = 0;
  for (const auto &buf : _buffers) {
    ret += buf.size();
  }
  return ret;
}

void BufferList::remove_prefix(size_t n) {
  while (n > 0) {
    if (_buffers.empty()) {
      throw std::out_of_range("BufferList::remove_prefix");
    }

    if (n < _buffers.front().str().size()) {
      _buffers.front().remove_prefix(n);
      n = 0;
    } else {
      n -= _buffers.front().str().size();
      _buffers.pop_front();
    }
  }
}

BufferViewList::BufferViewList(const BufferList &buffers) {
  for (const auto &x : buffers.buffers()) {
    _views.push_back(x);
  }
}

void BufferViewList::remove_prefix(size_t n) {
  while (n > 0) {
    if (_views.empty()) {
      throw std::out_of_range("BufferListView::remove_prefix");
    }

    if (n < _views.front().size()) {
      _views.front().remove_prefix(n);
      n = 0;
    } else {
      n -= _views.front().size();
      _views.pop_front();
    }
  }
}

size_t BufferViewList::size() const {
  size_t ret = 0;
  for (const auto &buf : _views) {
    ret += buf.size();
  }
  return ret;
}

// creates POSIX scatter/gather structures.
// Why important? Allows kernel syscalls like: writev() + sendmsg(),to send MANY buffers at once.
// Instead of: string packet = header + payload;  send(packet);which copies memory,
// kernel can do: writev(fd, iovecs, count); Zero-copy style. Very important high-performance networking technique.
// Each iovec describes one memory region.
// struct iovec {
//     void  *iov_base;
//     size_t iov_len;
// };


vector<iovec> BufferViewList::as_iovecs() const {
  vector<iovec> ret;
  ret.reserve(_views.size());
  for (const auto &x : _views) {
    // const_cast<char *>(x.data())
    // Because POSIX iovec API is ancient C API: 
    ret.push_back({const_cast<char *>(x.data()), x.size()});
  }
  return ret;
}
