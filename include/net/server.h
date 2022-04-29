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
      : server_(nullptr), state_(), error_() {}

  void Bind(const Address::Ptr& addr) {
    server_ = new TcpServer(addr);
  }

  void Start() {
    server_->Start();
  }

  int get_state() const { return state_; }
  int get_error() const { return error_; }

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

 protected:
  int state_;
  int error_;

};

using REQ = RpcProtocol;
using RESP = RpcProtocol;
//template <class REQ, class RESP>
class NetworkServer : public BaseServer {
 public:
  using Callback = std::function<void(const NetworkServer*)>&;

  NetworkServer(Callback cb)
      : cb_(cb) {}

 private:
  void OnNewConn(const TcpSocket::Ptr& new_sock) override {
    req_.set_socket(new_sock);
    req_.Recv();
    cb_(this);
    req_.Send();
  }

  REQ* req() { return &req_; }
  RESP* resp() { return &resp_; }

 private:
  REQ req_;
  RESP resp_;

  Callback cb_;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_SERVER_H
