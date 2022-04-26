//
// Created by titto on 2022/4/26.
//

#include <iostream>

#include "net/socket.h"
#include "scheduler.h"
#include "rpc/protocol.h"

using namespace tit::co;

void test_client() {
  IPv4Address::Ptr addr = IPv4Address::Create("127.0.0.1", 8888);
  TcpSocket socket;
  socket.CreateSocket();
  socket.Connect(addr);

  Protocol::Ptr protocol = Protocol::Create(MsgType::kRequestCall, 1, "hello world");
  std::string data = BaseProtocolSerializer::Serialize(protocol);
  socket.Send(std::to_string(data.size()).data(), 4, -1);
  socket.Send(data.data(), data.size(), -1);
  char buf[1024];
  socket.Recv(buf, 1024, -1);
  sleep(100000);
}

int main() {
  go(test_client);

  char ch;
  std::cin >> ch;
}
