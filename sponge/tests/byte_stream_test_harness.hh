#ifndef SPONGE_BYTE_STREAM_HARNESS_HH
#define SPONGE_BYTE_STREAM_HARNESS_HH

#include "byte_stream.hh"
#include "util.hh"

#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

/**
In C++, there is no functional difference between a struct and a class except for default access specifiers. The choice here is likely a coding convention preference:
- struct is typically used for simple data structures or interfaces
- class is typically used for more complex objects with behavior


What is virtual ? 
The virtual keyword in C++ enables polymorphism, which allows derived classes to override base class methods:
- When a function is marked virtual, calling it on a base class pointer/reference actually calls the derived class's implementation
- virtual ~ByteStreamTestStep() ensures proper cleanup when deleting derived class objects through base class pointers
- override keyword (used in derived classes) guarantees the function is actually overriding a virtual function from the base class

What is operator in C++? 
  operator is used to define operator overloading in C++. 
  It allows you to define custom behavior for operators when used with your class.

  `virtual operator std::string() const;`
  This is a conversion operator (also called a cast operator). 
  It allows objects of ByteStreamTestStep to be automatically converted to std::string.

Inheritance Hierarchy:
The code shows two main types of derived classes:

- ByteStreamAction: Performs operations on the byte stream (e.g., Write, Pop, EndInput)
- ByteStreamExpectation: Verifies properties of the byte stream (e.g., BufferSize, InputEnded)
This design pattern allows creating test sequences where different actions and expectations can be combined polymorphically through the common ByteStreamTestStep interface.

*/
struct ByteStreamTestStep {

  virtual operator std::string() const;

  // Defines how to run the test step on a ByteStream object
  // Each derived class implements its own behavior here
  virtual void execute(ByteStream &) const;

  // Ensures proper cleanup when objects are deleted via base class pointer
  // Critical for preventing memory leaks in polymorphic hierarchies
  virtual ~ByteStreamTestStep();
};

class ByteStreamExpectationViolation : public std::runtime_error {
public:
  ByteStreamExpectationViolation(const std::string &msg);

  template <typename T>
  static ByteStreamExpectationViolation
  property(const std::string &property_name, const T &expected,
           const T &actual);
};

struct ByteStreamExpectation : public ByteStreamTestStep {
  operator std::string() const override;
  virtual std::string description() const;
  virtual void execute(ByteStream &) const override;
  virtual ~ByteStreamExpectation() override;
};

struct ByteStreamAction : public ByteStreamTestStep {
  operator std::string() const override;
  virtual std::string description() const;
  virtual void execute(ByteStream &) const override;
  virtual ~ByteStreamAction() override;
};

struct EndInput : public ByteStreamAction {
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct Write : public ByteStreamAction {
  std::string _data;
  std::optional<size_t> _bytes_written{};

  Write(const std::string &data);
  Write &with_bytes_written(const size_t bytes_written);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct Pop : public ByteStreamAction {
  size_t _len;

  Pop(const size_t len);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct InputEnded : public ByteStreamExpectation {
  bool _input_ended;

  InputEnded(const bool input_ended);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct BufferEmpty : public ByteStreamExpectation {
  bool _buffer_empty;

  BufferEmpty(const bool buffer_empty);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct Eof : public ByteStreamExpectation {
  bool _eof;

  Eof(const bool eof);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct BufferSize : public ByteStreamExpectation {
  size_t _buffer_size;

  BufferSize(const size_t buffer_size);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct BytesWritten : public ByteStreamExpectation {
  size_t _bytes_written;

  BytesWritten(const size_t bytes_written);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct BytesRead : public ByteStreamExpectation {
  size_t _bytes_read;

  BytesRead(const size_t bytes_read);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct RemainingCapacity : public ByteStreamExpectation {
  size_t _remaining_capacity;

  RemainingCapacity(const size_t remaining_capacity);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

struct Peek : public ByteStreamExpectation {
  std::string _output;

  Peek(const std::string &output);
  std::string description() const override;
  void execute(ByteStream &) const override;
};

class ByteStreamTestHarness {
  std::string _test_name;
  ByteStream _byte_stream;
  std::vector<std::string> _steps_executed{};

public:
  ByteStreamTestHarness(const std::string &test_name, const size_t capacity);

  void execute(const ByteStreamTestStep &step);
};

#endif // SPONGE_BYTE_STREAM_HARNESS_HH
