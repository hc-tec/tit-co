//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_RPC_SERVER_PROVIDER_H
#define TIT_COROUTINE_RPC_SERVER_PROVIDER_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "net/server.h"
#include "protocol.h"
#include "interfaces/protocol_interface.h"
#include "rpc/rpc_protocol_handler.h"

namespace tit {

namespace co {


//void func(NetworkServer<RpcRequest, RpcResponse>* server) {
//  RpcRequest* req = server->req();
//  Protocol* protocol = static_cast<Protocol*>(req->req_protocol());
//  LOG(INFO) << protocol;
//  server->resp()->set_resp_protocol(nullptr);
//}


class RpcServerProvider {
 public:
  using Ptr = std::shared_ptr<RpcServerProvider>;
  using Func = std::function<void(NetworkServer<RpcRequest, RpcResponse>*)>;

  static Ptr Create(const Address::Ptr& addr) {
    return std::make_shared<RpcServerProvider>(addr);
  }

  explicit RpcServerProvider(const Address::Ptr& addr)
      : server_([this](NetworkServer<RpcRequest, RpcResponse>* server) {
          this->OnNewConn(server);
        }) {
    server_.Bind(addr);
  }

  bool BindRegistry(const Address::Ptr& addr);

  void RegisterHandler(const std::string& name, const Func& func) {
    handlers_[name] = func;
  }

  void Start() {
    server_.Start();
  }


  void OnNewConn(NetworkServer<RpcRequest, RpcResponse>* server);

 private:

 private:
  NetworkServer<RpcRequest, RpcResponse> server_;
  // register functions
  std::map<std::string, Func> handlers_;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_RPC_SERVER_PROVIDER_H
