//
// Created by titto on 2022/4/21.
//

#ifndef TIT_COROUTINE_CONN_H
#define TIT_COROUTINE_CONN_H

#include "address.h"
#include "interfaces/connection_interface.h"
#include "interfaces/server_interface.h"
#include "sock.h"

namespace tit {

namespace co {

class TcpConn : public ConnectionInterface {
 public:
  explicit TcpConn(int sock) : sock_(sock) {}
  ~TcpConn() override { this->Close(0); }

  int Recv(void* buf, int n, int ms) override {
    return co::recv(sock_, buf, n, ms);
  }

  int Recvn(void* buf, int n, int ms) override {
    return co::recvn(sock_, buf, n, ms);
  }

  int Send(const void* buf, int n, int ms) override {
    return co::send(sock_, buf, n, ms);
  }

  int Close(int ms) override {
    int r = sock_ != -1 ? co::close(sock_, ms) : 0;
    sock_ = -1;
    return r;
  }

  int Reset(int ms) override {
    int r = sock_ != -1 ? co::reset_tcp_socket(sock_, ms) : 0;
    sock_ = -1;
    return r;
  }

  int socket() override {
    return sock_;
  }

 private:
  int sock_;
};


class TcpConnFactory : public ConnFactory {
 public:
  using type = TcpConn::Ptr;
  TcpConn::Ptr Create(int fd) override {
    return std::make_shared<TcpConn>(fd);
  }
};

}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_CONN_H
