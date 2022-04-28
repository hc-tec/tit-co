//
// Created by titto on 2022/4/21.
//

#include "net/tcp.h"

#include <functional>

#include "scheduler.h"

namespace tit {

namespace co {


void TcpServer::Start() {
  InitSock();
  LOG(DEBUG) << "start server";
  go(std::bind(&TcpServer::Loop, this));
}

void TcpServer::Exit() {

}

void TcpServer::Loop() {
  LOG(DEBUG) << "enter start loop";
  while (!stop_) {
    TcpSocket::Ptr client_sock = sock_->Accept();

    if (delegate_) go(std::bind(&Delegate::OnNewConn,
                                delegate_,
                                client_sock));
//    LOG(DEBUG) << "Accept: " << sock_->remote_addr()->ToString();
  }
}

}  // namespace co

}  // namespace tit