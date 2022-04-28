//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_HELLO_H
#define TIT_COROUTINE_HELLO_H

#include <string>
#include <memory>

#include "def.h"
#include "interfaces/protocol_interface.h"
#include "rpc/protocol.h"
#include "rpc/protocol_manager.h"
#include "rpc/protos/gen/hello.pb.h"
#include "rpc/protos/gen/world.pb.h"

namespace tit {

namespace co {

struct HelloProtocol : public ProtocolInterface {
 public:
  using Ptr = HelloProtocol*;

  static Ptr Create() {
    return new HelloProtocol();
  }

  static Ptr Create(const std::string& _name) {
    return new HelloProtocol(_name);
  }

  HelloProtocol() {}

  HelloProtocol(const std::string& _name)
      : name(_name) {}

  std::string name;
};

struct WorldProtocol : public ProtocolInterface {
 public:
  using Ptr = WorldProtocol*;

  static Ptr Create() {
    return new WorldProtocol();
  }

  static Ptr Create(int32 result) {
    return new WorldProtocol(result);
  }

  WorldProtocol() {}

  WorldProtocol(int32 _result)
      : result(_result) {}

  int32 result;
};

constexpr char HELLO_PROTOCOL[] = "HELLO_PROTOCOL";

template <>
struct SvrName2Protocol<HELLO_PROTOCOL> {
  using REQ = HelloProtocol;
  using RES = WorldProtocol;
};

struct HelloSrvTraits : public SrvTraits {
 public:

  HelloSrvTraits() {
    ProtocolManagerSingleton::instance().Register("hello", this);
  }

  SerializerInterface* req_serializer() override {
    return new Serializer<HelloProtocol>;
  }

  SerializerInterface* res_serializer() override {
    return new Serializer<WorldProtocol>;
  }

};

static struct HelloSrvTraits a;

template<>
class SerializerHandler<HelloProtocol> {
 public:
  virtual std::string Serialize(ProtocolInterface::Ptr protocol_) {
    auto protocol = (HelloProtocol*)protocol_;
    HelloProtocolBuf buf;
    buf.set_name(protocol->name);
    return buf.SerializeAsString();
  }
  virtual HelloProtocol::Ptr Deserialize(const std::string& stream) {
    HelloProtocolBuf buf;
    buf.ParseFromString(stream);
    return HelloProtocol::Create(buf.name());
  }
};

template<>
class SerializerHandler<WorldProtocol> {
 public:
  virtual std::string Serialize(ProtocolInterface::Ptr protocol_) {
    auto protocol = (WorldProtocol*)protocol_;
    WorldProtocolBuf buf;
    buf.set_result(protocol->result);
    return buf.SerializeAsString();
  }
  virtual WorldProtocol::Ptr Deserialize(const std::string& stream) {
    WorldProtocolBuf buf;
    buf.ParseFromString(stream);
    return WorldProtocol::Create(buf.result());
  }
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_HELLO_H
