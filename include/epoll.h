//
// Created by titto on 2022/4/11.
//

#ifndef TIT_COROUTINE_EPOLL_H
#define TIT_COROUTINE_EPOLL_H


#include <sys/epoll.h>
#include <sys/eventfd.h>



namespace tit {

namespace co {

int createEventfd();

class Epoll {
 public:
  explicit Epoll(int sched_id);
  ~Epoll();

  bool add_ev_read(int fd, int32_t co_id);
  bool add_ev_write(int fd, int32_t co_id);
  void del_ev_read(int fd);
  void del_ev_write(int fd);
  void del_event(int fd);

  int wait(int ms);

  void signal(char c = 'x');

  const epoll_event& operator[](int i) const {
    return ev_[i];
  }

  int user_data(const epoll_event& ev) {
    return ev.data.fd;
  }

  bool is_wakeup_fd(const epoll_event& ev) {
    return wakeup_fd_ == ev.data.fd;
  }

  void handle_wakeup_ev();

  void close();

 private:
  int ep_;
  int wakeup_fd_;
  int sched_id_;
  epoll_event* ev_;
  bool signaled_;
};


}  // namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_EPOLL_H
