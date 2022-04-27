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

template <class Protocol>
class SerializerHandler {
 public:
  std::string Serialize(const typename Protocol::Ptr& protocol) {
    LOG(ERROR) << "must implement Serialize method for the serializer handler of protocol<" << typeid(protocol).name() << ">";
    return nullptr;
  }

  typename Protocol::Ptr Deserialize(const std::string& stream) {
    LOG(ERROR) << "must implement Deserialize method for the serializer handler of protocol";
    return nullptr;
  }
};

template <class Protocol>
struct serializer_handler_traits {
  using default_handler = SerializerHandler<Protocol>;
};


template <class Protocol, class SerializerHandler =
    typename serializer_handler_traits<Protocol>::default_handler>
class Serializer {
 public:
  static std::string Serialize(const typename Protocol::Ptr& protocol) {
    SerializerHandler handle;
    return handle.Serialize(protocol);
  }

  static typename Protocol::Ptr Deserialize(const std::string& stream) {
    SerializerHandler handle;
    return handle.Deserialize(stream);
  }
};



}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_SERIALIZER_INTERFACE_H
