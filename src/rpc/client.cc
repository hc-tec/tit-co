//
// Created by titto on 2022/4/27.
//

#include "rpc/client.h"

namespace tit {

namespace co {

bool RpcClient::Connect(const Address::Ptr& addr) {
  TcpSocket::Ptr sock = TcpSocket::Create();
  if (!sock->Connect(addr)) return false;
  session_ = RpcSession::Create(sock);
  go([this]() {
    this->SendLoop();
  });
  go([this]() {
    this->RecvLoop();
  });
  return true;
}

void RpcClient::SendLoop() {
  Protocol::Ptr protocol = nullptr;
  while (true) {
    send_channel_ >> protocol;
    LOG(DEBUG) << "send loop";
    if (timeout()) {
      LOG(ERROR) << "rpc info send timeout";
    } else {
      if (!session_->SendProtocol(protocol)) {
        LOG(ERROR) << "rpc info send error";
      }
    }
    protocol = nullptr;
  }
}

void RpcClient::RecvLoop() {
  Protocol::Ptr protocol = nullptr;
  while (true) {
    protocol = session_->RecvProtocol();
    LOG(DEBUG) << "recv loop";
    uint32 req_id = protocol->req_id();
    Channel<Protocol::Ptr> recv_channel;
    {
      MutexGuard g(mutex_);
      recv_channel = std::move(request_map_[req_id]);
      request_map_.erase(req_id);
    }
    recv_channel << protocol;
    protocol = nullptr;
  }

}

}  // namespace co

}  // namespace tit