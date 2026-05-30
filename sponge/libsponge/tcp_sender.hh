#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"
#include <queue>


// ─────────────────────────────────────────────
// Helper class: Retransmission Timer
// Defined here, used only by TCPSender
// Purpose: Encapsulates the logic of the retransmission timer, so that TCPSender can focus on TCP logic.
// "If I sent something and haven't heard back, retransmit it after RTO."
// ─────────────────────────────────────────────
class RetransmissionTimer {
  private:
    unsigned int _rto;
    unsigned int _initial_rto;
    size_t _elapsed{0}; // Time elapsed since the oldest outstanding segment was sent (in ms)
    bool _running{false}; // Is the retransmission timer currently running?

  public:
    RetransmissionTimer(unsigned int initial_rto):
      _rto(initial_rto), _initial_rto(initial_rto){};

    void start(){
      _running = true;
      _elapsed = 0;
    }

    void stop(){
      _running = false;
      _elapsed = 0;
    }
     void reset(){
      _elapsed = 0;
     }

     void tick(size_t ms){
      if(_running)_elapsed += ms;
     }

     bool expired() const {
      return _running && _elapsed >- _rto;
     }

     bool running() const{return _running;}

     void double_rto(){
      _rto *=2;
     }
     void reset_rto(){
      _rto = _initial_rto;
     }

};

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
private:
  //! our initial sequence number, the number for our SYN.
  WrappingInt32 _isn;

  //! outbound queue of segments that the TCPSender wants sent
  std::queue<TCPSegment> _segments_out{};

  //! retransmission timer for the connection
  unsigned int _initial_retransmission_timeout;

  //! outgoing stream of bytes that have not yet been sent
  ByteStream _stream;

  //! the (absolute) sequence number for the next byte to be sent
  uint64_t _next_seqno{0};

  bool _syn_sent{false}; //!< Has the SYN segment been sent?
  bool _fin_sent{false}; //!< Has the FIN segment been sent?


  // the next sequence number the receiver expects (= latest ackno)
  uint64_t _last_acked_seqno{0}; 

  // the window size of the remote receiver, update by ack_received()
  uint16_t _window_size{1}; 


  // Number of consecutive retransmissions that have occurred in a row
  // purpose   → tells TCPConnection when to give up and abort
  // increment → inside tick(), when timer expires AND window > 0
  // reset     → inside ack_received(), when new data is acked
  unsigned int _consecutive_retransmissions{0}; 

  // //!< Segments that have been sent but not yet acknowledged, along with the time they were sent (in ms)
  // The outstanding segments data structure must answer these questions:
  // 1. What is the earliest unacknowledged segment?  → for retransmission
  // 2. Which segments can be removed?                → when ACK arrives
  // 3. How many sequence numbers are in flight?      → for bytes_in_flight()
  // Why std::queue Is the Right Choice? TCP retransmits the oldest outstanding segment first. 
  // Segments are acknowledged in order (ACK is cumulative). 
  std::queue<TCPSegment> _outstanding_segments{}; 

  // bool _timer_running{false}; //!< Is the retransmission timer currently running?
  // size_t _time_elapsed{0}; //!< Time elapsed since the oldest outstanding segment was sent (in ms)
  RetransmissionTimer _retransmission_timer;

  


public:
  //! Initialize a TCPSender
  TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
            const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
            const std::optional<WrappingInt32> fixed_isn = {});

  //! \name "Input" interface for the writer
  //!@{
  ByteStream &stream_in() { return _stream; }
  const ByteStream &stream_in() const { return _stream; }
  //!@}

  //! \name Methods that can cause the TCPSender to send a segment
  //!@{

  //! \brief A new acknowledgment was received
  void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

  //! \brief Generate an empty-payload segment (useful for creating empty ACK
  //! segments)
  void send_empty_segment();

  //! \brief create and send segments to fill as much of the window as possible
  void fill_window();

  //! \brief Notifies the TCPSender of the passage of time
  void tick(const size_t ms_since_last_tick);
  //!@}

  //! \name Accessors
  //!@{

  //! \brief How many sequence numbers are occupied by segments sent but not yet
  //! acknowledged?
  //! \note count is in "sequence space," i.e. SYN and FIN each count for one
  //! byte (see TCPSegment::length_in_sequence_space())
  size_t bytes_in_flight() const;

  //! \brief Number of consecutive retransmissions that have occurred in a row
  unsigned int consecutive_retransmissions() const;

  //! \brief TCPSegments that the TCPSender has enqueued for transmission.
  //! \note These must be dequeued and sent by the TCPConnection,
  //! which will need to fill in the fields that are set by the TCPReceiver
  //! (ackno and window size) before sending.
  std::queue<TCPSegment> &segments_out() { return _segments_out; }
  //!@}

  //! \name What is the next sequence number? (used for testing)
  //!@{

  //! \brief absolute seqno for the next byte to be sent
  uint64_t next_seqno_absolute() const { return _next_seqno; }

  //! \brief relative seqno for the next byte to be sent
  WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
  //!@}
};

#endif // SPONGE_LIBSPONGE_TCP_SENDER_HH
