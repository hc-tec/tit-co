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


int main () {

  co::IPv4Address::Ptr addr = co::IPv4Address::Create("127.0.0.1", 8888);
  co::TcpServer server(addr);

  server.Start();

  char ch;
  std::cin >> ch;
}

