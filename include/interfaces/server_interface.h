//
// Created by titto on 2022/4/21.
//

#ifndef TIT_COROUTINE_SERVER_INTERFACE_H
#define TIT_COROUTINE_SERVER_INTERFACE_H

namespace tit {

namespace co {

class ServerInterface {
 public:
  ServerInterface() = default;

  virtual void Start() = 0;

  virtual void Exit() = 0;
};

}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_SERVER_INTERFACE_H
