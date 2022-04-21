//
// Created by titto on 2022/4/13.
//

#include "io_event.h"

#include "scheduler.h"

namespace tit {

namespace co {

IOEvent::~IOEvent() {
  auto& scheduler = TLSScheduler::instance();
  if (has_ev_) scheduler->DelIoEvent(fd_, ev_);
}

bool IOEvent::wait(uint32 ms) {
  auto& scheduler = TLSScheduler::instance();
  if (!has_ev_) {
    has_ev_ = scheduler->AddIoEvent(fd_, ev_);
    if (!has_ev_) return false;
  }
  if (ms != -1) {
    scheduler->AddTimer(ms);
    scheduler->Yield();
    if (!scheduler->is_timeout()) {
      return true;
    } else {
      errno = ETIMEDOUT;
      return false;
    }
  } else {
    scheduler->Yield();
    return true;
  }
}

}  // namespace co

}  // namespace tit