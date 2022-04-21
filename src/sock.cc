#ifndef _WIN32

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>


#include "io_event.h"
#include "scheduler.h"
#include "sock_ctx.h"
#include "sock.h"

#define CHECK(cond) \
    if (!(cond)) LOG(FATAL) << "check failed: " #cond "! "

namespace tit {
namespace co {

void set_nonblock(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void set_cloexec(int fd) {
  fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

#ifdef SOCK_NONBLOCK
int socket(int domain, int type, int protocol) {
  return ::socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
}

#else
int socket(int domain, int type, int protocol) {
  int fd = (socket)(domain, type, protocol);
  if (fd != -1) {
    co::set_nonblock(fd);
    co::set_cloexec(fd);
  }
  return fd;
}
#endif

int close(int fd, int ms) {
  auto& gSched = TLSScheduler::instance();
  if (fd < 0) return ::close(fd);
  if (gSched) {
    gSched->DelIoEvent(fd);
    if (ms > 0) gSched->Sleep(ms);
  } else {
    get_sock_ctx(fd).DelEvent();
  }

  int r;
  while ((r = ::close(fd)) != 0 && errno == EINTR);
  return r;
}

int shutdown(int fd, char c) {
  auto& gSched = TLSScheduler::instance();
  if (fd < 0) return ::shutdown(fd, SHUT_RDWR);

  if (gSched) {
    if (c == 'r') {
      gSched->DelIoEvent(fd, io_event_t::kEvRead);
      return ::shutdown(fd, SHUT_RD);
    } else if (c == 'w') {
      gSched->DelIoEvent(fd, io_event_t::kEvWrite);
      return ::shutdown(fd, SHUT_WR);
    } else {
      gSched->DelIoEvent(fd);
      return ::shutdown(fd, SHUT_RDWR);
    }

  } else {
    if (c == 'r') {
      co::get_sock_ctx(fd).DelEvRead();
      return ::shutdown(fd, SHUT_RD);
    } else if (c == 'w') {
      co::get_sock_ctx(fd).DelEvWrite();
      return ::shutdown(fd, SHUT_WR);
    } else {
      co::get_sock_ctx(fd).DelEvRead();
      return ::shutdown(fd, SHUT_RDWR);
    }
  }
}

int bind(int fd, const void* addr, int addrlen) {
  return ::bind(fd, (const struct sockaddr*)addr, (socklen_t)addrlen);
}

int listen(int fd, int backlog) { return ::listen(fd, backlog); }

int accept(int fd, void* addr, int* addrlen) {
  auto& gSched = TLSScheduler::instance();
  CHECK(gSched) << "must be called in coroutine..";
  IOEvent ev(fd, io_event_t::kEvRead);

  do {
#ifdef SOCK_NONBLOCK
    int connfd = accept4(fd, (sockaddr*)addr, (socklen_t*)addrlen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd != -1) return connfd;
#else
    int connfd = (accept)(fd, (sockaddr*)addr, (socklen_t*)addrlen);
    if (connfd != -1) {
      co::set_nonblock(connfd);
      co::set_cloexec(connfd);
      return connfd;
    }
#endif

    if (errno == EWOULDBLOCK || errno == EAGAIN) {
      ev.wait();
    } else if (errno != EINTR) {
      return -1;
    }
  } while (true);
}

int connect(int fd, const void* addr, int addrlen, int ms) {
  auto& gSched = TLSScheduler::instance();
  CHECK(gSched) << "must be called in coroutine..";
  do {
    int r = ::connect(fd, (const sockaddr*)addr, (socklen_t)addrlen);
    if (r == 0) return 0;

    if (errno == EINPROGRESS) {
      IOEvent ev(fd, io_event_t::kEvWrite);
      if (!ev.wait(ms)) return -1;

      int err, len = sizeof(err);
      r = co::getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
      if (r != 0) return -1;
      if (err == 0) return 0;
      errno = err;
      return -1;

    } else if (errno != EINTR) {
      return -1;
    }
  } while (true);
}

int recv(int fd, void* buf, int n, int ms) {
  auto& gSched = TLSScheduler::instance();
  CHECK(gSched) << "must be called in coroutine..";
  IOEvent ev(fd, io_event_t::kEvRead);

  do {
    int r = ::recv(fd, buf, n, 0);
    if (r != -1) return r;

    if (errno == EWOULDBLOCK || errno == EAGAIN) {
      if (!ev.wait(ms)) return -1;
    } else if (errno != EINTR) {
      return -1;
    }
  } while (true);
}

int recvn(int fd, void* buf, int n, int ms) {
  char* s = (char*)buf;
  int remain = n;
  IOEvent ev(fd, io_event_t::kEvRead);

  do {
    int r = ::recv(fd, s, remain, 0);
    if (r == remain) return n;
    if (r == 0) return 0;

    if (r == -1) {
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        if (!ev.wait(ms)) return -1;
      } else if (errno != EINTR) {
        return -1;
      }
    } else {
      remain -= r;
      s += r;
    }
  } while (true);
}

int recvfrom(int fd, void* buf, int n, void* addr, int* addrlen, int ms) {
  auto& gSched = TLSScheduler::instance();
  CHECK(gSched) << "must be called in coroutine..";
  IOEvent ev(fd, io_event_t::kEvRead);
  do {
    int r =
        ::recvfrom(fd, buf, n, 0, (sockaddr*)addr, (socklen_t*)addrlen);
    if (r != -1) return r;

    if (errno == EWOULDBLOCK || errno == EAGAIN) {
      if (!ev.wait(ms)) return -1;
    } else if (errno != EINTR) {
      return -1;
    }
  } while (true);
}

int send(int fd, const void* buf, int n, int ms) {
  auto& gSched = TLSScheduler::instance();
  CHECK(gSched) << "must be called in coroutine..";
  const char* s = (const char*)buf;
  int remain = n;
  IOEvent ev(fd, io_event_t::kEvWrite);

  do {
    int r = ::send(fd, s, remain, 0);
    if (r == remain) return n;

    if (r == -1) {
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        if (!ev.wait(ms)) return -1;
      } else if (errno != EINTR) {
        return -1;
      }
    } else {
      remain -= r;
      s += r;
    }
  } while (true);
}

int sendto(int fd, const void* buf, int n, const void* addr, int addrlen,
           int ms) {
  auto& gSched = TLSScheduler::instance();
  CHECK(gSched) << "must be called in coroutine..";
  const char* s = (const char*)buf;
  int remain = n;
  IOEvent ev(fd, io_event_t::kEvWrite);

  do {
    int r = ::sendto(fd, s, remain, 0, (const sockaddr*)addr,
                          (socklen_t)addrlen);
    if (r == remain) return n;

    if (r == -1) {
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        if (!ev.wait(ms)) return -1;
      } else if (errno != EINTR) {
        return -1;
      }
    } else {
      remain -= r;
      s += r;
    }
  } while (true);
}

}  // namespace co
}  // namespace tit
#endif
