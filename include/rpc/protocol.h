//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_PROTOCOL_H
#define TIT_COROUTINE_PROTOCOL_H


#include <memory>
#include <sstream>
#include <utility>

#include "def.h"
#include "interfaces/protocol_interface.h"
#include "interfaces/serializer_interface.h"

namespace tit {

namespace co {

enum MsgType : uint32 {
  kHeartBeat,           // 心跳包
  kProvider,            // 向服务中心声明为provider
  kConsumer,            // 向服务中心声明为consumer

  kRequest,             // 通用请求
  kResponse,            // 通用响应

  kRequestCall ,        // 请求方法调用
  kResponseCall,        // 响应方法调用

  kRegister,            // 向中心注册服务
  kRegisterResponse,

  kDiscover,            // 向中心请求服务发现
  kDiscoverResponse
};

/*
 * 私有通信协议
 * +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
 * |  BYTE  |        |        |        |        |        |        |        |        |        |        |             ........                                                           |
 * +--------------------------------------------+--------+--------------------------+--------+-----------------+--------+--------+--------+--------+--------+--------+-----------------+
 * |  magic | version|  type  |          sequence id              |          content length           |             content byte[]                                                     |
 * +--------+-----------------------------------------------------------------------------------------------------------------------------+--------------------------------------------+
 * 第一个字节是魔法数。
 * 第二个字节代表协议版本号，以便对协议进行扩展，使用不同的协议解析器。
 * 第三个字节是请求类型，如心跳包，rpc请求。
 * 第四个字节开始是一个32位序列号。
 * 第七个字节开始的四字节表示消息长度，即后面要接收的内容长度。
 */
class Protocol : public ProtocolInterface {
 public:
  using Ptr = std::shared_ptr<Protocol>;

  static constexpr uint32 kMagic = 0xaa;
  static constexpr uint32 kVersion = 0x01;
  static constexpr uint32 kBasicLen = 20;

  Protocol(MsgType type, uint32 req_id, std::string  data)
      : type_(type),
        req_id_(req_id),
        data_(std::move(data)) {}

  static Ptr Create(MsgType type, uint32 req_id, const std::string& data) {
    Ptr proto = std::make_shared<Protocol>(type, req_id, data);
    proto->set_data_len(data.size());
    return proto;
  }

  static Ptr CreateHeartBeat() {
    static Ptr heart_beat = Create(MsgType::kHeartBeat, 0, "");
    return heart_beat;
  }

  uint32 magic() const { return magic_; }
  uint32 version() const { return version_; }
  MsgType msg_type() const { return type_; }
  uint32_t req_id() const { return req_id_; }
  uint32_t data_len() const { return data_len_; }
  const std::string& data() const { return data_; }

  void set_magic(uint32 magic) { magic_ = magic; }
  void set_version(uint32 version) { version_ = version; }
  void set_msg_type(MsgType type) { type_ = type; }
  void set_req_id(uint32_t id) { req_id_ = id; }
  void set_data_len(uint32_t len) { data_len_ = len; }
  void set_data(const std::string& content) { data_ = content; }


  std::string toString() {
    std::stringstream ss;
    ss << "[ magic=" << magic_
       << " version=" << version_
       << " type=" << type_
       << " id=" << req_id_
       << " length=" << data_len_
       << " content=" << data_
       << " ]";
    return ss.str();
  }

 private:

  uint32 magic_ { kMagic };
  uint32 version_ { kVersion };
  MsgType type_;
  uint32 req_id_;
  uint32 data_len_ { kBasicLen };
  std::string data_;
};


class BaseProtocolHandler : public SerializerHandler<Protocol> {
 public:
  std::string Serialize(const Protocol::Ptr& protocol) override;
  Protocol::Ptr Deserialize(const std::string& stream) override;
};

template <>
class Serializer<Protocol, BaseProtocolHandler> {
 public:
  static std::string Serialize(const Protocol::Ptr& protocol) {
    BaseProtocolHandler handle;
    return handle.Serialize(protocol);
  }

  static Protocol::Ptr Deserialize(const std::string& stream) {
    BaseProtocolHandler handle;
    return handle.Deserialize(stream);
  }

};

using BaseProtocolSerializer = Serializer<Protocol, BaseProtocolHandler>;

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_PROTOCOL_H
