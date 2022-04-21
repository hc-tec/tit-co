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

void SockCtx::DelEvRead() {
  read_ev_.sched_id_ = -1;
  read_ev_.co_id_ = -1;
}

void SockCtx::DelEvWrite() {
  write_ev_.sched_id_ = -1;
  write_ev_.co_id_ = -1;
}

void SockCtx::DelEvent() {
  DelEvRead();
  DelEvWrite();
}

bool SockCtx::has_event() const {
  return has_ev_read() || has_ev_write();
}

bool SockCtx::has_ev_read() const {
  return read_ev_.co_id_ != -1;
}

bool SockCtx::has_ev_read(uint32 sched_id) const {
  return sched_id == read_ev_.sched_id_ &&
        read_ev_.co_id_ != -1;
}

bool SockCtx::has_ev_write() const {
  return write_ev_.co_id_ != -1;
}

bool SockCtx::has_ev_write(uint32 sched_id) const {
  return sched_id == write_ev_.sched_id_ &&
        write_ev_.co_id_ != -1;
}

uint32 SockCtx::get_read_co_id(uint32 sched_id) const {
  return sched_id == read_ev_.sched_id_ ? read_ev_.co_id_ : -1;
}

uint32 SockCtx::get_write_co_id(uint32 sched_id) const {
  return sched_id == write_ev_.sched_id_ ? write_ev_.co_id_ : -1;
}

SockCtx& get_sock_ctx(int fd) {
   static SockCtxMap map = SockCtxSingleton::instance();
   return map[fd];
}

}  // namespace co

}  // namespace tit