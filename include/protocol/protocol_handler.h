//
// Created by titto on 2022/4/29.
//

#ifndef TIT_COROUTINE_PROTOCOL_HANDLER_H
#define TIT_COROUTINE_PROTOCOL_HANDLER_H

#include "net/socket.h"
#include "protocol/protocol_message.h"
#include "protocol/protocol_transfer.h"

namespace tit {

namespace co {

class ProtocolHandler
    : public ProtocolMessage,
      public ProtocolTransfer {
 public:

  int get_state() const { return state_; }
  int get_error() const { return error_; }

  void Recv() {
    req_protocol_ = RecvProtocol();
    if (req_protocol_ == nullptr) {
      state_ = 1;
      error_ = 1;
    }
  }

  void Send() {
    SendProtocol(resp_protocol_);
  }

  ProtocolInterface* req_protocol() { return req_protocol_; }
  ProtocolInterface* resp_protocol() { return resp_protocol_; }

  void set_socket(const TcpSocket::Ptr& socket) { socket_ = socket; }

 protected:
  TcpSocket::Ptr socket_;
  ProtocolInterface* req_protocol_;
  ProtocolInterface* resp_protocol_;

 protected:
  int state_;
  int error_;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_PROTOCOL_HANDLER_H
