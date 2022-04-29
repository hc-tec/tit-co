//
// Created by titto on 2022/4/29.
//

#ifndef TIT_COROUTINE_RPC_PROTOCOL_HANDLER_H
#define TIT_COROUTINE_RPC_PROTOCOL_HANDLER_H

#include "def.h"

#include "interfaces/protocol_interface.h"
#include "protocol/protocol_handler.h"

namespace tit {

namespace co {

class RpcProtocol : public ProtocolHandler {
 public:

  RpcProtocol() {
      set_size_limit(1);
  }

  std::string Encode(ProtocolInterface::Ptr protocol) override;
  ProtocolInterface::Ptr Decode(const char *buf, size_t size) override;

  ProtocolInterface::Ptr RecvProtocol() override;

  bool SendProtocol(ProtocolInterface::Ptr protocol) override;

 public:
  static constexpr int8 kDataLen = 4;
  static constexpr int16 kDataMaxLen = 10000;
};

class RpcRequest : public RpcProtocol {

};

class RpcResponse: public RpcProtocol {

};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_RPC_PROTOCOL_HANDLER_H
