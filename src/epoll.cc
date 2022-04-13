//
// Created by titto on 2022/4/13.
//

#include "epoll.h"

#include <sys/eventfd.h>
#include <sys/socket.h>
#include <unistd.h>

#include "atomic.h"
#include "co_error.h"
#include "sock_ctx.h"
#include "log/logging.h"

namespace tit {

namespace co {

namespace {

int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG(ERROR) << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

}  // namespace

Epoll::Epoll(uint32 sched_id)
    : sched_id_(sched_id),
      wake_up_fd_(createEventfd()),
      is_wakeup_(false) {
  ep_ = epoll_create1(0);
  if (ep_ < 0) {
    LOG(FATAL) << "epoll create err";
    return;
  }
  ev_ = static_cast<epoll_event*>(calloc(kEventNum, sizeof(epoll_event)));
  if (ev_ == nullptr) {
    LOG(FATAL) << "epoll_event calloc err";
    return;
  }
  AddEvRead(wake_up_fd_, 0);
}

Epoll::~Epoll() {
  if (ev_) {
    free(ev_);
    ev_ = nullptr;
  }
  Close();
}

bool Epoll::AddEvRead(int fd, uint32 co_id) const {
  if (fd < 0) return false;
  auto& ctx = get_sock_ctx(fd);
  // fd read event have already add into coroutine
  if (ctx.has_ev_read()) return true;

  bool has_ev_write = ctx.has_ev_write(sched_id_);
  epoll_event ev{};
  ev.events = has_ev_write ? (EPOLLIN | EPOLLOUT | EPOLLET)
                           : (EPOLLIN | EPOLLET);
  ev.data.fd = fd;
  const int res = epoll_ctl(
      ep_, has_ev_write ? EPOLL_CTL_MOD
                        : EPOLL_CTL_ADD, fd, &ev);
  if (likely(res == 0)) {
    ctx.AddEvRead(sched_id_, co_id);
    return true;
  } else {
    LOG(ERROR) << "epoll_ctl read err";
    return false;
  }
}

bool Epoll::AddEvWrite(int fd, uint32 co_id) const {
  if (fd < 0) return false;
  auto& ctx = get_sock_ctx(fd);
  // fd read event have already add into coroutine
  if (ctx.has_ev_write()) return true;

  bool has_ev_read = ctx.has_ev_read(sched_id_);
  epoll_event ev{};
  ev.events = has_ev_read ? (EPOLLOUT | EPOLLIN | EPOLLET)
                           : (EPOLLOUT | EPOLLET);
  ev.data.fd = fd;
  const int res = epoll_ctl(
      ep_, has_ev_read ? EPOLL_CTL_MOD
                        : EPOLL_CTL_ADD, fd, &ev);
  if (likely(res == 0)) {
    ctx.AddEvWrite(sched_id_, co_id);
    return true;
  } else {
    LOG(ERROR) << "epoll_ctl write err";
    return false;
  }
}

void Epoll::DelEvRead(int fd, uint32 co_id) const {
  if (fd < 0) return;
  auto& ctx = get_sock_ctx(fd);
  // fd read event have already add into coroutine
  if (!ctx.has_ev_read()) return;
  int res;
  ctx.DelEvRead();
  if (!ctx.has_ev_write()) {
    res = epoll_ctl(ep_, EPOLL_CTL_DEL, fd, nullptr);
  } else {
    epoll_event ev{};
    ev.events = EPOLLOUT | EPOLLET;
    res = epoll_ctl(ep_, EPOLL_CTL_MOD, fd, &ev);
  }
  if (res != 0 && errno != ENOENT) {
    LOG(ERROR) << "epoll del ev_read error: " << ", fd: " << fd;
  }
}

void Epoll::DelEvWrite(int fd, uint32 co_id) const {
  if (fd < 0) return;
  auto& ctx = get_sock_ctx(fd);
  // fd read event have already add into coroutine
  if (!ctx.has_ev_write()) return;
  int res;
  ctx.DelEvWrite();
  if (!ctx.has_ev_read()) {
    res = epoll_ctl(ep_, EPOLL_CTL_DEL, fd, nullptr);
  } else {
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    res = epoll_ctl(ep_, EPOLL_CTL_MOD, fd, &ev);
  }
  if (res != 0 && errno != ENOENT) {
    LOG(ERROR) << "epoll del ev_write error: " << ", fd: " << fd;
  }
}

void Epoll::DelEvent(int fd) const {
  if (fd < 0) return;
  auto& ctx = get_sock_ctx(fd);
  if (ctx.has_event()) {
    ctx.DelEvent();
    const int res = epoll_ctl(ep_, EPOLL_CTL_DEL, fd, nullptr);
    if (res != 0) LOG(ERROR) << "epoll del event error: " << ", fd: " << fd;
  }
}

bool Epoll::is_wakeup_fd(const epoll_event& ev) const {
  return ev.data.fd == wake_up_fd_;
}

void Epoll::Wakeup() {
  uint64 one = 1;
  if (atomic_compare_swap(&is_wakeup_, false, true) == false) {
    while (true) {
      const int res = write(wake_up_fd_, &one, sizeof(one));
      if (res != -1) {
        if (res < 10) break;
        continue;
      } else {
        if (errno == EWOULDBLOCK || errno == EAGAIN) break;
        if (errno == EINTR) continue;
        LOG(ERROR) << "wake up fd: " << wake_up_fd_ << " error, " << stderror();
        break;
      }
    }

  }
}

void Epoll::HandleWakeUpEvent() {
  uint64 one;
  while (true) {
    int res = read(wake_up_fd_, &one, sizeof(one));
    if (res != -1) {
      if (res < 10) break;
      continue;
    } else {
      if (errno == EWOULDBLOCK || errno == EAGAIN) break;
      if (errno == EINTR) continue;
      LOG(ERROR) << "pipe read error: "<< ", fd: " << wake_up_fd_;
      break;
    }
  }
  atomic_swap(&is_wakeup_, false);
}

void Epoll::Close() const {
  close(wake_up_fd_);
  close(ep_);
}

int Epoll::Wait(int ms) {
  return epoll_wait(ep_, ev_, kEventNum, ms);
}

}  // namespace co

}  // namespace tit
