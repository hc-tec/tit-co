//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_SERIALIZER_INTERFACE_H
#define TIT_COROUTINE_SERIALIZER_INTERFACE_H

#include <string>

#include "log/logging.h"
#include "protocol_interface.h"

namespace tit {

namespace co {

class SerializerInterface {
 public:
  virtual std::string Serialize(ProtocolInterface::Ptr protocol) { return nullptr; }
  virtual ProtocolInterface::Ptr Deserialize(const std::string& stream) { return nullptr; }
};

template <typename Protocol>
class SerializerHandler : public SerializerInterface {
 public:
  virtual std::string Serialize(typename Protocol::Ptr protocol) {
    LOG(ERROR) << "must implement Serialize method for the serializer handler of protocol<" << typeid(protocol).name() << ">";
    return nullptr;
  }

  virtual typename Protocol::Ptr Deserialize(const std::string& stream) {
    LOG(ERROR) << "must implement Deserialize method for the serializer handler of protocol";
    return nullptr;
  }
};

template <typename Protocol>
struct serializer_handler_traits {
  using default_handler = SerializerHandler<Protocol>;
};


template <typename Protocol, typename SerializerHandler =
    typename serializer_handler_traits<Protocol>::default_handler>
class Serializer : public SerializerInterface {
 public:
  std::string Serialize(ProtocolInterface::Ptr protocol) override {
    SerializerHandler handle;
    return handle.Serialize(protocol);
  }

  ProtocolInterface::Ptr Deserialize(const std::string& stream) override {
    SerializerHandler handle;
    return handle.Deserialize(stream);
  }
};



}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_SERIALIZER_INTERFACE_H
