//
// Created by titto on 2022/4/29.
//

#include "rpc/rpc_protocol_handler.h"

#include <rpc/protocol.h>

namespace tit {

namespace co {

std::string RpcProtocol::Encode(ProtocolInterface* protocol) {
  BaseProtocolSerializer serializer;
  std::string buf = serializer.Serialize(protocol);
  int32 buf_len = buf.size();
  if (buf_len > kDataMaxLen) {
    LOG(DEBUG) << "total data length can't greaten than " << kDataMaxLen;
    return "";
  }
  char data_len[4];
  snprintf(data_len, 5, "%04d",buf_len);
  return std::string(data_len, 4).append(buf);
}

ProtocolInterface* RpcProtocol::Decode(const char *buf,
                                                  size_t size) {
  BaseProtocolSerializer serializer;
  return serializer.Deserialize(std::string(buf));
}

ProtocolInterface* RpcProtocol::RecvProtocol() {
  LOG(DEBUG) << "recv protocol";
  char data_len_[kDataLen+1];
  data_len_[kDataLen] = '\0';
  int read = socket_->Recvn(data_len_, kDataLen, -1);
  if (read == 0) {
    socket_->Close();
    return nullptr;
  }
  if(read < kDataLen) {
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
}

bool RpcProtocol::SendProtocol(ProtocolInterface* protocol) {
  if (protocol == nullptr) return false;
  LOG(DEBUG) << "send protocol";
  std::string buf = Encode(protocol);
  uint buf_len = buf.size();
  if (buf_len == 0) {
    LOG(DEBUG) << "total data length can't greaten than " << kDataMaxLen;
    return false;
  }
  LOG(DEBUG) << "send protocol buf size: " << buf_len;
  LOG(DEBUG) << "send protocol buf data: " << buf;
  if (socket_->Send(buf.data(), buf_len, -1) < buf_len) {
    LOG(DEBUG) << "packet body send error";
    return false;
  }
  return true;
}

}  // namespace co

}  // namespace tit