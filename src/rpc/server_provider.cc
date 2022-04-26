//
// Created by titto on 2022/4/26.
//

#include "rpc/server_provider.h"

namespace tit {

namespace co {

void ServerProvider::OnNewConn(const TcpSocket::Ptr& new_sock) {

}

bool ServerProvider::BindRegistry(const Address::Ptr& addr) { return false; }

}  // namespace co

}  // namespace tit