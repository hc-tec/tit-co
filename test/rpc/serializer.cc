//
// Created by titto on 2022/4/25.
//

#include <string>
#include <iostream>

#include "test.pb.h"

int main() {
  TestForProto proto;
  proto.add_id(1);
  proto.set_name("titto");
  proto.set_password("123456");

  std::string str = proto.SerializeAsString();
  std::cout << str << std::endl;

  TestForProto proto2;

  proto2.ParseFromString(str);
  std::cout << proto2.name() << ' ' << proto2.password();
}
