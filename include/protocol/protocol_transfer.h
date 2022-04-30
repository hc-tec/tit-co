//
// Created by titto on 2022/4/29.
//

#ifndef TIT_COROUTINE_PROTOCOL_TRANSFER_H
#define TIT_COROUTINE_PROTOCOL_TRANSFER_H

#include "interfaces/protocol_interface.h"

namespace tit {

namespace co {

class ProtocolTransfer {
 public:

  virtual ProtocolInterface* RecvProtocol() = 0;

  virtual bool SendProtocol(ProtocolInterface* protocol) = 0;

};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_PROTOCOL_TRANSFER_H
