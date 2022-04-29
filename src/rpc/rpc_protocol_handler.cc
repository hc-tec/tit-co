//
// Created by titto on 2022/4/29.
//

#include "rpc/rpc_protocol_handler.h"

#include <rpc/protocol.h>

namespace tit {

namespace co {

std::string RpcProtocol::Encode(ProtocolInterface::Ptr protocol) {
  BaseProtocolSerializer serializer;
  std::string buf = serializer.Serialize(protocol);
  int32 buf_len = buf.size();
  if (buf_len > kDataMaxLen) {
    LOG(DEBUG) << "total data length can't greaten than " << kDataMaxLen;
    return "";
  }
  char data_len[4];
  snprintf(data_len, 4, "%04d",buf_len);
  return std::string(data_len, 4).append(buf);
}

ProtocolInterface::Ptr RpcProtocol::Decode(const char *buf,
                                                  size_t size) {
  BaseProtocolSerializer serializer;
  return serializer.Deserialize(std::string(buf));
}

ProtocolInterface::Ptr RpcProtocol::RecvProtocol() {
  LOG(DEBUG) << "recv protocol";
  char data_len_[kDataLen+1];
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

  return static_cast<Protocol*>(Decode(buf, data_len));
//  std::string data(buf);
//  LOG(DEBUG) << "recv protocol buf data: " << buf;
//  BaseProtocolSerializer serializer;
//  Protocol::Ptr protocol = static_cast<Protocol*>(serializer.Deserialize(data));
//  return protocol;
}

bool RpcProtocol::SendProtocol(ProtocolInterface::Ptr protocol) {
  constexpr int8 kDataMaxLen = kDataLen;
  LOG(DEBUG) << "send protocol";
  std::string buf = Encode(protocol);
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