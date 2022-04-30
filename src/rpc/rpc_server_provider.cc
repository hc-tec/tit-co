//
// Created by titto on 2022/4/26.
//

#include "rpc/rpc_server_provider.h"
#include "rpc/protocol_manager.h"

namespace tit {

namespace co {

void RpcServerProvider::OnNewConn(
    NetworkServer<RpcRequest, RpcResponse>* server) {

  RpcRequest* req = server->req();
  if (req->get_error() == 1) {
    return;
  }
  Protocol* protocol = static_cast<Protocol*>(req->req_protocol());
  LOG(INFO) << protocol->toString();

  if (protocol->msg_type() == kRequestCall) {
    Protocol::Ptr base_protocol;
    std::string srv_name = protocol->srv_name();
    auto pair = handlers_.find(srv_name);
    if (pair != handlers_.end()) {
      SrvTraits* traits = ProtocolManagerSingleton::instance().Get(srv_name);
      req->DeserializeDataReqProtocol(traits, srv_name, protocol->data());

      Func func = pair->second;
      func(server);

      RpcResponse* resp = server->resp();
      base_protocol = Protocol::Create(kResponseCall, protocol->req_id(), "");
      resp->set_resp_protocol(base_protocol);
      resp->SerializeDataRespProtocol(traits);

    } else {
      RpcResponse* resp = server->resp();
      base_protocol = Protocol::Create(kResponseCall, protocol->req_id(), "other");
      resp->set_resp_protocol(base_protocol);
    }
  }

}

bool RpcServerProvider::BindRegistry(const Address::Ptr& addr) { return false; }

}  // namespace co

}  // namespace tit