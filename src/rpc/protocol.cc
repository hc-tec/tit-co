//
// Created by titto on 2022/4/26.
//

#include "rpc/protocol.h"
#include "rpc/protos/base.pb.h"

namespace tit {

namespace co {

std::string BaseProtocolHandler::Serialize(const Protocol::Ptr& protocol) {
  BaseProtocolBuf buf;
  buf.set_magic(protocol->magic());
  buf.set_version(protocol->version());
  buf.set_type(protocol->msg_type());
  buf.set_data_len(protocol->data_len());
  buf.set_data(protocol->data());
  return buf.SerializeAsString();
}

Protocol::Ptr BaseProtocolHandler::Deserialize(const std::string& stream) {
  BaseProtocolBuf buf;
  buf.ParseFromString(stream);
  return Protocol::Create(
      static_cast<MsgType>(buf.type()),
      buf.req_id(),
      buf.data()
      );
}

}  // namespace co

}  // namespace tit