//
// Created by titto on 2022/4/27.
//

#ifndef TIT_COROUTINE_RPC_H
#define TIT_COROUTINE_RPC_H

#include "def.h"
#include "protocol_manager.h"
#include "protocols/hello.h"

namespace tit {

namespace co {


enum RpcState : uint8 {
  kSuccess,
  kFail,
  kNoMethod,
  kClosed,
  kTimeout
};

template <typename ...>
class Result;

template <class BaseProtocol>
class Result<BaseProtocol> {
 public:
  using BaseProtocolPtr = typename BaseProtocol::Ptr;
  void set_state(RpcState state) { state_ = state; }
  void set_base_proto(const BaseProtocolPtr& proto) {
    base_proto_ = proto;
  }

  RpcState state() { return state_; }
  BaseProtocolPtr base_proto() { return base_proto_; }

 private:
  BaseProtocolPtr base_proto_;
  RpcState state_;
};

template <class BaseProtocol, class TargetProtocol>
class Result<BaseProtocol, TargetProtocol> {
 public:
  using BaseProtocolPtr = typename BaseProtocol::Ptr;
  using TargetProtocolPtr = typename TargetProtocol::Ptr;

  Result() {}

  Result(const BaseProtocolPtr& base_proto,
         const TargetProtocolPtr& target_proto)
      : base_proto_(base_proto),
        target_proto_(target_proto),
        state_(kSuccess) {}

  auto& Success() { state_ = kSuccess; return *this; }
  auto& Fail() { state_ = kFail; return *this; }
  auto& NoMethod() { state_ = kNoMethod; return *this; }
  auto& Closed() { state_ = kClosed; return *this; }
  auto& Timeout() { state_ = kTimeout; return *this; }

  void set_state(RpcState state) { state_ = state; }
  void set_base_proto(const BaseProtocolPtr& proto) {
    base_proto_ = proto;
  }
  void set_target_proto(const TargetProtocolPtr& proto) {
    target_proto_ = proto;
  }

  RpcState state() { return state_; }
  BaseProtocolPtr base_proto() { return base_proto_; }
  TargetProtocolPtr target_proto() { return target_proto_; }
 private:
  BaseProtocolPtr base_proto_;
  TargetProtocolPtr target_proto_;
  RpcState state_;
};

}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_RPC_H
