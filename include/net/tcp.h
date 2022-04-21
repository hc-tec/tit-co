//
// Created by titto on 2022/4/21.
//

#ifndef TIT_COROUTINE_TCP_H
#define TIT_COROUTINE_TCP_H

#include "conn.h"
#include "socket.h"

namespace tit {

namespace co {

class TcpServer : public ServerInterface {
 public:

  explicit TcpServer(const Address::Ptr& addr)
      : stop_(false) {
    addr_ = addr;
    InitSock();
  }

  TcpServer(const char* ip, uint16 port)
      : stop_(false) {
    addr_ = IPv4Address::Create(ip, port);
    InitSock();
  }

  ~TcpServer() {

  };
  void Start() override;
  void Exit() override;

 private:

  void InitSock() {
    sock_.Bind(addr_);
    sock_.Listen();
  }

  void Loop();

  Address::Ptr addr_;
  TcpSocket sock_;

  bool stop_;
};


}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_TCP_H
