//
// Created by titto on 2022/4/27.
//

#ifndef TIT_COROUTINE_CLIENT_H
#define TIT_COROUTINE_CLIENT_H

#include <map>
#include <string>

#include "atomic.h"
#include "channel.h"
#include "mutex.h"
#include "net/address.h"
#include "net/socket.h"
#include "protocol.h"
#include "protocol_manager.h"
#include "rpc.h"
#include "rpc_protocol_handler.h"

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

  template <char const* srv_proto>
  Result<typename SvrName2Protocol<srv_proto>::REQ,
         typename SvrName2Protocol<srv_proto>::RES>
      Call(const std::string& srv_name,
       typename SvrName2Protocol<srv_proto>::REQ::Ptr req_proto) {
    using REQProtocol = typename SvrName2Protocol<srv_proto>::REQ;
    using RESProtocol = typename SvrName2Protocol<srv_proto>::RES;
    using REQProtocolPtr = typename REQProtocol::Ptr;
    using RESProtocolPtr = typename RESProtocol::Ptr;

    Serializer<REQProtocol> serializer;
    std::string target_data = serializer.Serialize((ProtocolInterface*)req_proto);

    Protocol::Ptr base_proto = Protocol::Create(MsgType::kRequestCall, req_id_, target_data);
    base_proto->set_srv_name(srv_name);
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
      Result<REQProtocol , RESProtocol> res;
      return res.Timeout();
    }

    Serializer<RESProtocol> res_serializer;
    RESProtocolPtr res_proto = (RESProtocolPtr) res_serializer.Deserialize(recv_proto->data());

    if (res_proto == nullptr) {
      Result<REQProtocol , RESProtocol> res;
      return res.NoMethod();
    }
    Result<REQProtocol , RESProtocol> res(req_proto, res_proto);
    return res.Success();
  }

 private:
  void SendLoop();
  void RecvLoop();

 private:
  RpcProtocol protocol_handler_;

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
