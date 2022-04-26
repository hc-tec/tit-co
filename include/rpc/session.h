//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_SESSION_H
#define TIT_COROUTINE_SESSION_H

#include <memory>

#include "protocol.h"
#include "net/socket.h"

namespace tit {

namespace co {

class RpcSession {
 public:
  using Ptr = std::shared_ptr<RpcSession>;

  static Ptr Create(const TcpSocket::Ptr& socket) {
    return std::make_shared<RpcSession>(socket);
  }

  explicit RpcSession(const TcpSocket::Ptr& socket)
      : socket_(socket) {}

  Protocol::Ptr RecvProtocol();

  bool SendProtocol(const Protocol::Ptr& protocol);

 private:
 TcpSocket::Ptr socket_;

};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_SESSION_H
