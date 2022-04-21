//
// Created by titto on 2022/4/21.
//

#include "scheduler.h"

#include <iostream>

#include "channel.h"
#include "interfaces/connection.h"
#include "log/logging.h"
#include "net/tcp.h"
#include "sock.h"
#include "net/address.h"
#include "net/socket.h"

using namespace tit;

class ServerHandler : public co::TcpServer::Delegate {
 public:
  void OnBind(const co::TcpSocket::Ptr &server_sock) override {
    LOG(INFO) << "Bind";
  }
  void OnListen(const co::TcpSocket::Ptr &server_sock) override {
    LOG(INFO) << "Listen";
  }
  void OnNewConn(const co::TcpSocket::Ptr &new_sock) override {
    LOG(INFO) << "New Connection";
  }
  void OnMessage(const co::TcpSocket::Ptr &new_sock, const char *buffer,
                 uint len) override {

  }
  void OnSendFinish(const co::TcpSocket::Ptr &new_sock, const char *buffer,
                    uint len) override {

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

