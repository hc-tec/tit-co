//
// Created by titto on 2022/4/21.
//

#include <iostream>

#include "channel.h"
#include "interfaces/connection_interface.h"
#include "log/logging.h"
#include "net/address.h"
#include "net/socket.h"
#include "net/tcp.h"
#include "scheduler.h"
#include "sock.h"

using namespace tit;

class ServerHandler : public co::TcpServer::Delegate {
 public:
  void OnBind(const co::TcpSocket::Ptr &server_sock, const co::Address::Ptr& addr) override {
    LOG(INFO) << "Bind";
  }
  void OnListen(const co::TcpSocket::Ptr &server_sock) override {
    LOG(INFO) << "Listen";
  }
  void OnNewConn(const co::TcpSocket::Ptr &new_sock) override {
    LOG(INFO) << "New Connection";
  }
};

int main () {

  co::IPv4Address::Ptr addr = co::IPv4Address::Create("127.0.0.1", 8889);
  co::TcpServer server(addr);
  server.set_delegate(new ServerHandler());
  server.Start();

  char ch;
  std::cin >> ch;
}

