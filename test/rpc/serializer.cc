//
// Created by titto on 2022/4/25.
//

#include <string>
#include <iostream>

#include "rpc/protocol.h"
#include "rpc/serializer.h"

using namespace tit::co;

int main() {

  Protocol::Ptr protocol = Protocol::Create(MsgType::kRequestCall, 123, "wohuqifei");

  std::string binary_data = BaseProtocolSerializer::Serialize(protocol);

  std::cout << binary_data << std::endl;

  Protocol::Ptr parsed_protocol = BaseProtocolSerializer::Deserialize(binary_data);
  std::cout << parsed_protocol->version() << std::endl;
}
