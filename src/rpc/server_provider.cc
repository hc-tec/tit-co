//
// Created by titto on 2022/4/26.
//

#include "rpc/protocol.h"
#include "rpc/session.h"
#include "rpc/server_provider.h"

namespace tit {

namespace co {

void RpcServerProvider::OnNewConn(const TcpSocket::Ptr& new_sock) {
  RpcSession::Ptr session = RpcSession::Create(new_sock);
  Protocol::Ptr protocol = session->RecvProtocol();
  LOG(INFO) << "end";
  new_sock->Close();
}

bool RpcServerProvider::BindRegistry(const Address::Ptr& addr) { return false; }

}  // namespace co

}  // namespace tit