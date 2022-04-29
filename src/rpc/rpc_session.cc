//
// Created by titto on 2022/4/26.
//

#include "rpc/rpc_session.h"

#include <algorithm>

#include "log/logging.h"
#include "rpc/rpc_protocol_handler.h"

namespace tit {

namespace co {

Protocol::Ptr RpcSession::RecvProtocol() {
  constexpr int8 kDataLen = RpcProtocolMessage::kDataLen;
  LOG(DEBUG) << "recv protocol";
  char data_len_[RpcProtocolMessage::kDataLen+1];
  data_len_[kDataLen] = '\0';
  if(socket_->Recvn(data_len_, kDataLen, -1) < kDataLen) {
    return nullptr;
  }
  int data_len = atoi(data_len_);
  if (data_len == 0) return nullptr;

  LOG(DEBUG) << "packet length: " << data_len;

  char buf[data_len+1];
  buf[data_len] = '\0';
  if(socket_->Recvn(buf, data_len, -1) < data_len) {
    return nullptr;
  }
  LOG(DEBUG) << "recv protocol buf data: " << buf;
  return static_cast<Protocol*>(message_in()->Decode(buf, data_len));
//  std::string data(buf);
//  LOG(DEBUG) << "recv protocol buf data: " << buf;
//  BaseProtocolSerializer serializer;
//  Protocol::Ptr protocol = static_cast<Protocol*>(serializer.Deserialize(data));
//  return protocol;
}

bool RpcSession::SendProtocol(ProtocolInterface::Ptr protocol) {
  constexpr int8 kDataMaxLen = RpcProtocolMessage::kDataLen;
  LOG(DEBUG) << "send protocol";
  std::string buf = message_out()->Encode(protocol);
  uint buf_len = buf.size();
  if (buf_len == 0) {
    LOG(DEBUG) << "total data length can't greaten than " << kDataMaxLen;
    return false;
  }
//  BaseProtocolSerializer serializer;
//  std::string buf = serializer.Serialize(protocol);
//  int32 buf_len = buf.size();
//  if (buf_len > kDataMaxLen) {
//    LOG(DEBUG) << "total data length can't greaten than " << kDataMaxLen;
//    return false;
//  }
//  std::string data_len = std::to_string(buf_len);
//
//  if (socket_->Send(data_len.data(), kDataLen, -1) < kDataLen) {
//    LOG(DEBUG) << "packet length send error";
//    return false;
//  }
  LOG(DEBUG) << "send protocol buf size: " << buf_len;
  LOG(DEBUG) << "send protocol buf data: " << buf.data();
  if (socket_->Send(buf.data(), buf_len, -1) < buf_len) {
    LOG(DEBUG) << "packet body send error";
    return false;
  }
  return true;
}

}  // namespace co

}  // namespace tit