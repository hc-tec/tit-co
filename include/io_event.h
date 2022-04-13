//
// Created by titto on 2022/4/13.
//

#ifndef TIT_COROUTINE_IO_EVENT_H
#define TIT_COROUTINE_IO_EVENT_H

namespace tit {

namespace co {

enum io_event_t {
  kEvRead = 1,
  kEvWrite = 2,
};
class IOEvent {};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_IO_EVENT_H
