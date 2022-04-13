//
// Created by titto on 2022/4/13.
//

#include "sock_ctx.h"

#include <cstring>

namespace tit {

namespace co {

void SockCtx::AddEvRead(uint32 sched_id, uint32 co_id) {
  read_ev_.sched_id_ = sched_id;
  read_ev_.co_id_ = co_id;
}

void SockCtx::AddEvWrite(uint32 sched_id, uint32 co_id) {
  write_ev_.sched_id_ = sched_id;
  write_ev_.co_id_ = co_id;
}

void SockCtx::DelEvRead(uint32 sched_id, uint32 co_id) {
  read_clear_ = 0;
}

void SockCtx::DelEvWrite(uint32 sched_id, uint32 co_id) {
  write_clear_ = 0;
}

void SockCtx::DelEvent(uint32 sched_id) {
  memset(this, 0, sizeof(*this));
}

bool SockCtx::has_event() const {
  return has_ev_read() || has_ev_write();
}

bool SockCtx::has_ev_read() const {
  return read_ev_.co_id_ != 0;
}

bool SockCtx::has_ev_read(uint32 sched_id) const {
  return sched_id == read_ev_.sched_id_ &&
        read_ev_.co_id_ != 0;
}

bool SockCtx::has_ev_write() const {
  return write_ev_.co_id_ != 0;
}

bool SockCtx::has_ev_write(uint32 sched_id) const {
  return sched_id == write_ev_.sched_id_ &&
        write_ev_.co_id_ != 0;
}

uint32 SockCtx::read_co_id(uint32 sched_id) const {
  return sched_id == read_ev_.sched_id_ ? read_ev_.co_id_ : 0;
}

uint32 SockCtx::write_co_id(uint32 sched_id) const {
  return sched_id == write_ev_.sched_id_ ? write_ev_.co_id_ : 0;
}

}  // namespace co

}  // namespace tit