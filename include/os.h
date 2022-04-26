//
// Created by titto on 2022/4/26.
//

#ifndef TIT_COROUTINE_OS_H
#define TIT_COROUTINE_OS_H

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <cstring>
#include <map>

namespace tit {

namespace co {

typedef void (*sig_handler_t)(int);

class FailureHandler {
 public:
  FailureHandler();
  ~FailureHandler();

  void on_signal(int sig);
  void on_failure(int sig);

 private:
  std::map<int, sig_handler_t> _old_handlers;
};

struct Global {
  Global();
  ~Global() = delete;
  FailureHandler* failure_handler;
};

inline Global& global() {
  static Global* g = new Global();
  return *g;
}

sig_handler_t signal(int sig, sig_handler_t handler, int flag = 0);

void on_signal_(int sig);  // handler for SIGINT SIGTERM SIGQUIT
void on_failure_(int sig); // handler for SIGSEGV SIGABRT SIGFPE SIGBUS SIGILL


}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_OS_H
