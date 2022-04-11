//
// Created by titto on 2022/4/11.
//

#include "log/logging.h"

#include "def.h"
#include "epoll.h"
#include "sock_ctx.h"

namespace tit {

namespace co {


int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
  //    LOG_SYSERR << "Failed in eventfd";
  abort();
  }
  return evtfd;
}

Epoll::Epoll(int sched_id)
    : ep_(epoll_create1(0)),
      wakeup_fd_(createEventfd()),
      sched_id_(sched_id),
      signaled_(false) {
  ev_ = (epoll_event*)calloc(1024, sizeof(epoll_event));
}

Epoll::~Epoll() {
  close();
  if (ev_) {
    free(ev_);
    ev_ = nullptr;
  }
}

bool Epoll::add_ev_read(int fd, int32_t co_id) {
  if (fd < 0) return false;
  auto& ctx = co::get_sock_ctx(fd);
  if (ctx.has_ev_read()) return true;

  const bool has_ev_write = ctx.has_ev_write(sched_id_);
  epoll_event ev;
  ev.events = has_ev_write ? (EPOLLIN | EPOLLOUT | EPOLLET) : (EPOLLIN | EPOLLET);
  ev.data.fd = fd;
  const int res = epoll_ctl(ep_, has_ev_write ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &ev);
  // success
  if (likely(res == 0)) {
    ctx.add_ev_read(sched_id_, co_id);
    return true;
  } else {
    LOG(ERROR) << "epoll add ev read error: " << "fd: " << fd << ", co: " << co_id;
    return false;
  }
}

bool Epoll::add_ev_write(int fd, int32_t co_id) {
  if (fd < 0) return false;
  auto& ctx = co::get_sock_ctx(fd);
  if (ctx.has_ev_write()) return true;

  const bool has_ev_read = ctx.has_ev_read(sched_id_);
  epoll_event ev;
  ev.events = has_ev_read ? (EPOLLIN | EPOLLOUT | EPOLLET) : (EPOLLOUT | EPOLLET);
  ev.data.fd = fd;
  const int res = epoll_ctl(ep_, has_ev_read ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &ev);
  // success
  if (likely(res == 0)) {
    ctx.add_ev_write(sched_id_, co_id);
    return true;
  } else {
    LOG(ERROR) << "epoll add ev write error: " << "fd: " << fd << ", co: " << co_id;
    return false;
  }
}

void Epoll::del_ev_read(int fd) {
  if (fd < 0) return;
  auto& ctx = co::get_sock_ctx(fd);
  if (!ctx.has_ev_read()) return; // not exists

  int r;
  ctx.del_ev_read();
  if (!ctx.has_ev_write(sched_id_)) {
    r = epoll_ctl(ep_, EPOLL_CTL_DEL, fd, (epoll_event*)8);
  } else {
    epoll_event ev;
    ev.events = EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    r = epoll_ctl(ep_, EPOLL_CTL_MOD, fd, &ev);
  }

  if (r != 0 && errno != ENOENT) {
    LOG(ERROR) << "epoll del ev_read error: " << ", fd: " << fd;
  }
}

void Epoll::del_ev_write(int fd) {
  if (fd < 0) return;
  auto& ctx = co::get_sock_ctx(fd);
  if (!ctx.has_ev_write()) return; // not exists

  int r;
  ctx.del_ev_write();
  if (!ctx.has_ev_read(sched_id_)) {
    r = epoll_ctl(ep_, EPOLL_CTL_DEL, fd, (epoll_event*)8);
  } else {
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    r = epoll_ctl(ep_, EPOLL_CTL_MOD, fd, &ev);
  }

  if (r != 0 && errno != ENOENT) {
    LOG(ERROR) << "epoll del ev_write error: " << ", fd: " << fd;
  }
}

void Epoll::del_event(int fd) {
  if (fd < 0) return;
  auto& ctx = co::get_sock_ctx(fd);
  if (ctx.has_event()) {
    ctx.del_event();
    const int r = epoll_ctl(ep_, EPOLL_CTL_DEL, fd, (epoll_event*)8);
    if (r != 0) LOG(ERROR) << "epoll del event error: " << ", fd: " << fd;
  }
}

void Epoll::handle_wakeup_ev() {
  int32_t dummy;

}
}  // namespace co

}  // namespace tit
