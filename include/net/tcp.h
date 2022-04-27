//
// Created by titto on 2022/4/21.
//

#ifndef TIT_COROUTINE_TCP_H
#define TIT_COROUTINE_TCP_H

#include <memory>
#include <utility>

#include "conn.h"
#include "socket.h"

namespace tit {

namespace co {

class TcpServer : public ServerInterface {
 public:
  using Ptr = std::shared_ptr<TcpServer>;

  class Delegate {
   public:
    virtual void OnBind(const TcpSocket::Ptr& server_sock, const Address::Ptr& addr) = 0;
    virtual void OnListen(const TcpSocket::Ptr& server_sock) = 0;
    virtual void OnNewConn(const TcpSocket::Ptr& new_sock) = 0;
  };

  explicit TcpServer(const Address::Ptr& addr)
      : addr_(addr),
        sock_(TcpSocket::Create(addr->family(), kTcp, 0)),
        delegate_(nullptr),
        stop_(false) {
  }

  TcpServer(const char* ip, uint16 port)
      : addr_(IPv4Address::Create(ip, port)),
        sock_(TcpSocket::Create(addr_->family(), kTcp, 0)),
        delegate_(nullptr),
        stop_(false) {
  }

  ~TcpServer() {

  };

  static Ptr Create(const Address::Ptr& addr) {
    return std::make_shared<TcpServer>(addr);
  }

  static Ptr Create(const char* ip, uint16 port) {
    return std::make_shared<TcpServer>(ip, port);
  }

  void Start() override;
  void Exit() override;

  void set_delegate(Delegate* delegate) {
    delegate_ = delegate;
  }

 private:

  void InitSock() {
    sock_->Bind(addr_);
    if (delegate_) delegate_->OnBind(sock_, addr_);
    sock_->Listen();
    if (delegate_) delegate_->OnListen(sock_);
  }

  void Loop();

  Address::Ptr addr_;
  TcpSocket::Ptr sock_;

  Delegate* delegate_;

  bool stop_;
};


}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_TCP_H
