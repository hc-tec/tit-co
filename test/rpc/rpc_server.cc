//
// Created by titto on 2022/4/25.
//

#include <iostream>
#include <string>

#include "rpc/protocols/hello.h"
#include "rpc/rpc_server_provider.h"

using namespace tit::co;

ProtocolInterface::Ptr hello(ProtocolInterface::Ptr req) {
  auto req_protocol = (HelloProtocol::Ptr) req;
  WorldProtocol::Ptr res_protocol = WorldProtocol::Create(req_protocol->name.size());
  return res_protocol;
}

int main() {
  IPv4Address::Ptr addr = IPv4Address::Create("127.0.0.1", 8888);
  RpcServerProvider::Ptr serverProvider = RpcServerProvider::Create(addr);

  serverProvider->RegisterHandler("hello", hello);

  serverProvider->Start();

  char ch;
  std::cin >> ch;
}
