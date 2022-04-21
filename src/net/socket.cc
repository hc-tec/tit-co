//
// Created by titto on 2022/4/21.
//

#include "net/socket.h"

namespace tit {

namespace co {

void Socket::InitLocalAddr() {
  if (local_addr_) return;
  switch (family_) {
    case AF_INET:
      local_addr_.reset(new IPv4Address());
      break;
    case AF_INET6:
      local_addr_.reset(new IPv6Address());
      break;
  }
  socklen_t addrlen = local_addr_->addrlen();
  if (getsockname(fd_, local_addr_->addr(), &addrlen)) {
    LOG(ERROR) << "unknown address";
  }
}

void Socket::InitRemoteAddr() {
  if (remote_addr_) return;
  switch (family_) {
    case AF_INET:
      remote_addr_.reset(new IPv4Address());
      break;
    case AF_INET6:
      remote_addr_.reset(new IPv6Address());
      break;
  }
  socklen_t addrlen = remote_addr_->addrlen();
  if (getsockname(fd_, remote_addr_->addr(), &addrlen)) {
    LOG(ERROR) << "unknown address";
  }
}

TcpClientSocket::Ptr TcpServerSocket::Accept() {
  CHECK(is_valid());
  TcpClientSocket::Ptr sock = TcpClientSocket::Create(family(), type(), protocol());
  int client_sock = co::accept(fd(), nullptr, nullptr);
  if (client_sock < 0) {
    LOG(ERROR) << "accept error";
  }
  sock->Init(client_sock);
  return sock;
}

bool TcpClientSocket::Connect(const Address::Ptr& address,
                              uint64_t ms) {
  CHECK(is_valid());
  if(co::connect(fd(), address->addr(), address->addrlen(), ms)) {
    LOG(ERROR) << "connect error";
    return false;
  }

  InitLocalAddr();
  InitRemoteAddr();
  return true;
}

}  // namespace co

}  // namespace tit