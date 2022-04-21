//
// Created by titto on 2022/4/21.
//

#ifndef TIT_COROUTINE_CONNECTION_H
#define TIT_COROUTINE_CONNECTION_H

#include <memory>

namespace tit {

namespace co {


class ConnectionInterface {
 public:
  using Ptr = std::shared_ptr<ConnectionInterface>;

  ConnectionInterface() = default;
  virtual ~ConnectionInterface() = default;

  virtual int Recv(void* buf, int n, int ms) = 0;
  virtual int Recvn(void* buf, int n, int ms) = 0;
  virtual int Send(const void* buf, int n, int ms) = 0;

  virtual int Close(int ms) = 0;
  virtual int Reset(int ms) = 0;

  virtual int socket() = 0;
};


class ConnFactory {
 public:
  virtual ConnectionInterface::Ptr Create(int fd) {}
};

}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_CONNECTION_H
