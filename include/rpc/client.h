//
// Created by titto on 2022/4/27.
//

#ifndef TIT_COROUTINE_CLIENT_H
#define TIT_COROUTINE_CLIENT_H

#include <string>
#include <map>

#include "channel.h"
#include "session.h"
#include "mutex.h"
#include "rpc.h"
#include "protocol.h"
#include "atomic.h"
#include "net/socket.h"
#include "net/address.h"

namespace tit {

namespace co {

class RpcClient {
 public:
  using Ptr = std::shared_ptr<RpcClient>;
  using CoMutex = Mutex;

  typedef struct {
    uint32 recv_timeout_;
    uint32 send_timeout_;
    int8 send_channel_size;
  } Params;

  static constexpr Params kDefaultParams = {
      .recv_timeout_ = static_cast<uint32>(-1),
      .send_timeout_ = static_cast<uint32>(-1),
      .send_channel_size = 10
  };

  RpcClient() : RpcClient(kDefaultParams) {}

  RpcClient(const Params& params)
      : req_id_(1),
        params_(params),
        closed_(false),
        send_channel_(params_.send_channel_size, params_.send_timeout_) {}

  bool Connect(const Address::Ptr& addr);

//  template <class TargetProtocol>
  using TargetProtocol = Protocol;
  Result<Protocol, TargetProtocol> Call(const std::string& svr_name, const typename TargetProtocol::Ptr& target_proto) {

    Serializer<TargetProtocol> serializer;
    std::string target_data = serializer.Serialize(target_proto);

    Protocol::Ptr base_proto = Protocol::Create(MsgType::kRequestCall, req_id_, target_data);

    send_channel_ << base_proto;

    Channel<Protocol::Ptr> recv_channel_(1, params_.recv_timeout_);

    {
      MutexGuard g(mutex_);
      request_map_[req_id_] = recv_channel_;
      ++req_id_;
    }

    Protocol::Ptr recv_proto;
    recv_channel_ >> recv_proto;
    if (timeout()) {
      Result<Protocol, TargetProtocol> res;
      return res.Timeout();
    }
    Result<Protocol, TargetProtocol> res(base_proto, recv_proto);
    return res.Success();
  }

 private:
  void SendLoop();
  void RecvLoop();

 private:
  RpcSession::Ptr session_;

  uint32 req_id_;
  Params params_;
  bool closed_;
  CoMutex mutex_;
  Channel<Protocol::Ptr> send_channel_;
  std::map<uint32, Channel<Protocol::Ptr>> request_map_;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_CLIENT_H
