//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_SERIALIZER_INTERFACE_H
#define TIT_COROUTINE_SERIALIZER_INTERFACE_H

#include <string>

#include "protocol_interface.h"

namespace tit {

namespace co {

template <class Protocol>
class SerializerHandler {
 public:
  virtual std::string Serialize(const typename Protocol::Ptr& protocol) = 0;

  virtual typename Protocol::Ptr Deserialize(const std::string& stream) = 0;
};

template <class Protocol, class SerializerHandler>
class Serializer;

}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_SERIALIZER_INTERFACE_H
