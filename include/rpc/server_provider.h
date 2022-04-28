//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_SERVER_PROVIDER_H
#define TIT_COROUTINE_SERVER_PROVIDER_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "net/tcp.h"
#include "protocol.h"
#include "interfaces/protocol_interface.h"

namespace tit {

namespace co {

class RpcServerProvider : public TcpServer::Delegate {
 public:
  using Ptr = std::shared_ptr<RpcServerProvider>;
  using RpcReq = ProtocolInterface::Ptr;
  using RpcRes = ProtocolInterface::Ptr;
  using Func = std::function<RpcRes (const RpcReq&)>;

  static Ptr Create(const Address::Ptr& addr) {
    return std::make_shared<RpcServerProvider>(addr);
  }

  explicit RpcServerProvider(const Address::Ptr& addr)
      : server_(addr) {
    server_.set_delegate(this);
  }

  bool BindRegistry(const Address::Ptr& addr);

  void RegisterHandler(const std::string& name, const Func& func) {
    handlers_[name] = func;
  }

  void Start() {
    server_.Start();
  }

  void OnBind(const TcpSocket::Ptr& server_sock, const Address::Ptr& addr) override {}
  void OnListen(const TcpSocket::Ptr& server_sock) override {}
  void OnNewConn(const TcpSocket::Ptr& new_sock) override;
 private:

 private:
  TcpServer server_;
  // register functions
  std::map<std::string, Func> handlers_;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_SERVER_PROVIDER_H
