//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_SESSION_H
#define TIT_COROUTINE_SESSION_H

#include <net/session.h>

#include <memory>

#include "net/session.h"
#include "net/socket.h"
#include "protocol.h"

namespace tit {

namespace co {

class RpcSession : public Session {
 public:
  using Ptr = std::shared_ptr<RpcSession>;

  static Ptr Create(const TcpSocket::Ptr& socket) {
    return std::make_shared<RpcSession>(socket);
  }

  explicit RpcSession(const TcpSocket::Ptr& socket)
      : socket_(socket) {}

  Protocol::Ptr RecvProtocol();

  bool SendProtocol(Protocol::Ptr protocol);

 private:
  TcpSocket::Ptr socket_;

};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_SESSION_H
