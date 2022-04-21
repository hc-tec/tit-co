//
// Created by titto on 2022/4/13.
//

#ifndef TIT_COROUTINE_IO_EVENT_H
#define TIT_COROUTINE_IO_EVENT_H

#include "def.h"
#include "base/noncopyable.h"

namespace tit {

namespace co {

enum io_event_t {
  kEvRead = 1,
  kEvWrite = 2,
};

class IOEvent {
 public:
  IOEvent(int fd, io_event_t ev)
      : fd_(fd),
        ev_(ev),
        has_ev_(false) {}

  ~IOEvent();

  bool wait(uint32 ms = -1);

 private:
  DISALLOW_COPY_AND_ASSIGN(IOEvent);

  int fd_;
  io_event_t ev_;
  bool has_ev_;
};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_IO_EVENT_H
