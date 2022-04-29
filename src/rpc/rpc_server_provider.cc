//
// Created by titto on 2022/4/26.
//

#include "rpc/rpc_server_provider.h"

#include "rpc/protocol_manager.h"
#include "rpc/protocols/hello.h"

namespace tit {

namespace co {

void RpcServerProvider::OnNewConn(
    NetworkServer<RpcRequest, RpcResponse>* server) {

  RpcRequest* req = server->req();
  Protocol* protocol = static_cast<Protocol*>(req->req_protocol());
  LOG(INFO) << protocol->toString();
  server_.req()->set_resp_protocol(nullptr);
//  RpcSession::Ptr session = RpcSession::Create(new_sock);
//  Protocol::Ptr protocol = session->RecvProtocol();
//  if (protocol->msg_type() == kRequestCall) {
//    Protocol::Ptr base_protocol;
//    std::string srv_name = protocol->srv_name();
//    auto pair = handlers_.find(srv_name);
//    if (pair != handlers_.end()) {
//      Func func = pair->second;
//
//      SrvTraits* traits = ProtocolManagerSingleton::instance().Get(srv_name);
//      SerializerInterface* req_serializer = traits->req_serializer();
//      ProtocolInterface::Ptr req_proto = req_serializer->Deserialize(protocol->data());
//      ProtocolInterface::Ptr res_proto = func(req_proto);
//      SerializerInterface* res_serializer = traits->res_serializer();
//      std::string data = res_serializer->Serialize(res_proto);
//      base_protocol = Protocol::Create(kResponseCall, protocol->req_id(), data);
//
//      delete req_serializer;
//      delete res_serializer;
//      req_serializer = nullptr;
//      res_serializer = nullptr;
//      traits = nullptr;
//    } else {
//      base_protocol = Protocol::Create(kResponseCall, protocol->req_id(), "");
//    }
//    session->SendProtocol(base_protocol);
//  }
////  session->SendProtocol(protocol);
//  LOG(INFO) << protocol->toString();
////  new_sock->Close();
}

bool RpcServerProvider::BindRegistry(const Address::Ptr& addr) { return false; }

}  // namespace co

}  // namespace tit