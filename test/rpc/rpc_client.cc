//
// Created by titto on 2022/4/26.
//

#include "rpc/rpc_client.h"

#include <iostream>

#include "net/socket.h"
#include "rpc/protocol.h"
#include "rpc/protocols/hello.h"
#include "scheduler.h"

using namespace tit::co;

void test_client() {

  HelloProtocol* protocol = HelloProtocol::Create("hello world");

  IPv4Address::Ptr addr = IPv4Address::Create("127.0.0.1", 8888);
  RpcClient client;
  client.Connect(addr);
  Result<HelloProtocol, WorldProtocol> res = client.Call<HELLO_PROTOCOL>("hello", protocol);
  LOG(INFO) << res.base_proto()->name;
  LOG(INFO) << res.target_proto()->result;
}

void test2_client() {

  HelloProtocol* protocol = HelloProtocol::Create("test2");

  IPv4Address::Ptr addr = IPv4Address::Create("127.0.0.1", 8888);
  RpcClient client;
  client.Connect(addr);
  Result<HelloProtocol, WorldProtocol> res = client.Call<HELLO_PROTOCOL>("hello", protocol);
  LOG(INFO) << res.base_proto()->name;
  LOG(INFO) << res.target_proto()->result;
}


int main() {

  go(test_client);
  go(test2_client);

  char ch;
  std::cin >> ch;
}
