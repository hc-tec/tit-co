//
// Created by titto on 2022/4/25.
//

#include <string>
#include <iostream>

#include "os.h"
#include "rpc/server_provider.h"

using namespace tit::co;

int main() {
  IPv4Address::Ptr addr = IPv4Address::Create("127.0.0.1", 8888);
  RpcServerProvider::Ptr serverProvider = RpcServerProvider::Create(addr);
  serverProvider->Start();

  char ch;
  std::cin >> ch;
}
