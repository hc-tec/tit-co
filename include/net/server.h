//
// Created by titto on 2022/4/29.
//

#ifndef TIT_COROUTINE_SERVER_H
#define TIT_COROUTINE_SERVER_H

#include "net/tcp.h"
#include "rpc/rpc_protocol_handler.h"

namespace tit {

namespace co {


class BaseServer : public TcpServer::Delegate {
 public:
  BaseServer()
      : server_(nullptr) {}

  void Bind(const Address::Ptr& addr) {
    server_ = new TcpServer(addr);
    server_->set_delegate(this);
  }

  void Start() {
    server_->Start();
  }



 private:
  void OnBind(const TcpSocket::Ptr& server_sock, const Address::Ptr& addr) override {}
  void OnListen(const TcpSocket::Ptr& server_sock) override {}
  void OnNewConn(const TcpSocket::Ptr& new_sock) override {}

 protected:
  virtual ~BaseServer() {
    delete server_;
  }

 private:
  TcpServer* server_;

};

template <class REQ, class RESP>
class NetworkServer : public BaseServer {
 public:
  using Callback = std::function<void(NetworkServer<REQ, RESP>*)>;

  NetworkServer<REQ, RESP>(Callback cb)
      : cb_(std::move(cb)) {}

  REQ* req() { return &req_; }
  RESP* resp() { return &resp_; }

  void set_callback(Callback cb) {
    cb_ = std::move(cb);
  }

 private:
  void OnNewConn(const TcpSocket::Ptr& new_sock) override {
    req_.set_socket(new_sock);
    resp_.set_socket(new_sock);
    req_.Recv();
    cb_(this);
    if (req_.get_error() != 1) {
      resp_.Send();
    }
  }



 private:
  REQ req_;
  RESP resp_;

  Callback cb_;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_SERVER_H
