//
// Created by titto on 2022/4/21.
//

#ifndef TIT_COROUTINE_SOCKET_H
#define TIT_COROUTINE_SOCKET_H

#include "sock.h"
#include "address.h"
#include "tcp.h"

namespace tit {

namespace co {

static constexpr int kInvalidFd = -1;

class Socket {
 public:
  using Ptr = std::shared_ptr<Socket>;

  enum Type {
    kTcp = SOCK_STREAM,
    kUdp = SOCK_DGRAM
  };

  enum Family {
    kIpv4 = AF_INET,
    kIpv6 = AF_INET6
  };

  Socket(Family family, Type type, int protocol, bool create = true)
      : fd_(kInvalidFd),
        family_(family),
        type_(type),
        protocol_(protocol),
        connected_(false) {
    if (create) {
      fd_ = socket(family, type, protocol);
      Init(fd_);
    }
  }

  static Ptr Create(Family family, Type type, int protocol) {
    return std::make_shared<Socket>(family, type, protocol, false);
  }

  void Init(int fd) {
    if (unlikely(fd == kInvalidFd)) {
      LOG(ERROR) << "socket create error";
    } else {
      fd_ = fd;
      set_reuseaddr(fd_);
      conn_ = conn_factory_.Create(fd_);
      InitLocalAddr();
      InitRemoteAddr();
    }
  }

  virtual int Recv(void* buf, int n, int ms) {
    CHECK(connected_);
    return conn_->Recv(buf, n, ms);
  }

  virtual int Recvn(void* buf, int n, int ms) {
    CHECK(connected_);
   return conn_->Recvn(buf, n, ms);
  }

  virtual int Send(const void* buf, int n, int ms) {
    CHECK(connected_);
    return conn_->Send(buf, n, ms);
  }

  virtual bool Close(int ms) {
    if (!connected_) return true;
    connected_ = false;
    return conn_->Close(ms) == 0;
  }

  virtual int Reset(int ms) {
    CHECK(connected_);
    return conn_->Reset(ms);
  }

  int fd() const { return fd_; }
  Family family() { return family_; }
  Type type() { return type_; }
  int protocol() const { return protocol_; }
  bool connected() const { return connected_; }

  bool is_valid() const { return fd_ != kInvalidFd; }

  void InitLocalAddr();
  void InitRemoteAddr();

  Address::Ptr local_addr() { return local_addr_; }
  Address::Ptr remote_addr() { return remote_addr_; }

 private:
  int fd_;
  Family family_;
  Type type_;
  int protocol_;


  ConnFactory conn_factory_;
  ConnFactory::type conn_;

  Address::Ptr local_addr_;
  Address::Ptr remote_addr_;

 protected:
  bool connected_;
};

class TcpClientSocket : public Socket {
 public:
  using Ptr = std::shared_ptr<TcpClientSocket>;

  TcpClientSocket(Family family,
                  Type type,
                  int protocol,
                  bool create = true)
                  : Socket(family,type,protocol,create) {}

  static Ptr Create(Family family, Type type, int protocol) {
    return std::make_shared<TcpClientSocket>(family, type, protocol, false);
  }

  bool Connect(const Address::Ptr& address, uint64_t timeout_ms = -1);

};

class TcpServerSocket : public Socket {
 public:
  using Ptr = std::shared_ptr<TcpServerSocket>;

  bool Bind(const Address::Ptr& addr) {
    CHECK(is_valid());
    if (co::bind(fd(), addr->addr(), addr->addrlen())) {
      LOG(ERROR) << "bind error";
      return false;
    }
    return true;
  }

  bool Listen(int backlog = kMaxConn) {
    CHECK(is_valid());
    if (co::listen(fd(), backlog)) {
      LOG(ERROR) << "listen error";
      return false;
    }
    return true;
  }

  TcpClientSocket::Ptr Accept();

 private:
  static const int kMaxConn = 4096;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_SOCKET_H
