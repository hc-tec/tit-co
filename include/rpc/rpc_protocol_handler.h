//
// Created by titto on 2022/4/29.
//

#ifndef TIT_COROUTINE_RPC_PROTOCOL_HANDLER_H
#define TIT_COROUTINE_RPC_PROTOCOL_HANDLER_H

#include "def.h"

#include "interfaces/protocol_interface.h"
#include "protocol/protocol_handler.h"
#include "rpc/protocol_manager.h"

namespace tit {

namespace co {

class RpcProtocol : public ProtocolHandler {
 public:

  RpcProtocol() {
//      set_size_limit(1);
  }

  std::string Encode(ProtocolInterface* protocol) override;
  ProtocolInterface* Decode(const char *buf, size_t size) override;

  ProtocolInterface* RecvProtocol() override;

  bool SendProtocol(ProtocolInterface* protocol) override;

 public:
  static constexpr int8 kDataLen = 4;
  static constexpr int16 kDataMaxLen = 10000;
};

class RpcRequest : public RpcProtocol {
 public:

  RpcRequest()
      : data_req_protocol_(nullptr) {}

  void DeserializeDataReqProtocol(
      SrvTraits* traits,
      const std::string& srv_name,
      const std::string& raw_data) {
    SerializerInterface* req_serializer = traits->req_serializer();
    data_req_protocol_ = req_serializer->Deserialize(raw_data);
    delete req_serializer;
    req_serializer = nullptr;
  }

  ProtocolInterface* data_req_protocol() {
    return data_req_protocol_;
  }

 private:
  ProtocolInterface* data_req_protocol_;

};

class RpcResponse: public RpcProtocol {
 public:

  RpcResponse()
      : data_resp_protocol_(nullptr) {}

  void set_resp_protocol(ProtocolInterface* resp_protocol) {
    resp_protocol_ = resp_protocol;
  }

  void SerializeDataRespProtocol(SrvTraits* traits) {
    SerializerInterface* resp_serializer = traits->res_serializer();
    std::string data = resp_serializer->Serialize(data_resp_protocol_);
    ((Protocol*)resp_protocol_)->set_data(data);
    delete resp_serializer;
    resp_serializer = nullptr;
  }

  void set_data_resp_protocol(ProtocolInterface* data_resp_protocol) {
    data_resp_protocol_ = data_resp_protocol;
  }

  ProtocolInterface* data_resp_protocol() {
    return data_resp_protocol_;
  }

 private:
  ProtocolInterface* data_resp_protocol_;

};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_RPC_PROTOCOL_HANDLER_H
