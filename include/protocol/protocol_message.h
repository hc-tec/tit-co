//
// Created by titto on 2022/4/29.
//

#ifndef TIT_COROUTINE_PROTOCOL_MESSAGE_H
#define TIT_COROUTINE_PROTOCOL_MESSAGE_H

#include <string>

#include "def.h"
#include "interfaces/protocol_interface.h"

namespace tit {

namespace co {

class MessageOut {
 public:
  virtual ~MessageOut() {}
  virtual std::string Encode(ProtocolInterface* protocol) = 0;
};

class MessageIn {
 public:
  virtual ~MessageIn() {}
  virtual ProtocolInterface* Decode(const char* buf, size_t size) = 0;
};

class ProtocolMessage : public MessageOut, public MessageIn {

 public:
  ProtocolMessage()
      : size_limit_((uint64)-1) {}

  ~ProtocolMessage() override {

  }

 public:
  void set_size_limit(uint64 limit) { size_limit_ = limit; }
  uint64 size_limit() const { return size_limit_; }

 protected:
  uint64 size_limit_;

};

class ProtocolMessageWrapper : public ProtocolMessage {
 public:
  ProtocolMessageWrapper(ProtocolMessage* inner)
      : inner_(inner) {}

  std::string Encode(ProtocolInterface* protocol) override {
    std::string inner_data = ProtocolMessage::Encode(protocol);
    ProtocolInterface* inner_protocol = InnerDecode(inner_data.data(), inner_data.size());
    if (inner_) {
      inner_->Encode(inner_protocol);
    }
  }

  ProtocolInterface* Decode(const char* buf, size_t size) override {
    ProtocolInterface* inner_protocol = ProtocolMessage::Decode(buf, size);
    std::string inner_data = InnerEncode(inner_protocol);
    if (inner_) {
      inner_->Decode(inner_data.data(), inner_data.size());
    }
  }

 protected:
  virtual std::string InnerEncode(ProtocolInterface* protocol) {}
  virtual ProtocolInterface* InnerDecode(const char* buf, size_t size) {}

 private:
  ProtocolMessage* inner_;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_PROTOCOL_MESSAGE_H
