//
// Created by titto on 2022/4/26.
//

#include "os.h"

#include "log/logging.h"

namespace tit {

namespace co {

sig_handler_t signal(int sig, sig_handler_t handler, int flag) {
  struct sigaction sa, old{};
  memset(&sa, 0, sizeof(sa));
  sigemptyset(&sa.sa_mask);
  if (flag > 0) sa.sa_flags = flag;
  sa.sa_handler = handler;
  int r = sigaction(sig, &sa, &old);
  return r == 0 ? old.sa_handler : SIG_ERR;
}

Global::Global() {
  failure_handler = new FailureHandler();
}



void on_signal_(int sig) {
  global().failure_handler->on_signal(sig);
}

void on_failure_(int sig) {
  global().failure_handler->on_failure(sig);
}

FailureHandler::FailureHandler() {
  _old_handlers[SIGINT] = signal(SIGINT, on_signal_);
  _old_handlers[SIGTERM] = signal(SIGTERM, on_signal_);

#ifdef _WIN32
  _old_handlers[SIGABRT] = signal(SIGABRT, on_failure);
    // Signal handler for SIGSEGV and SIGFPE installed in main thread does
    // not work for other threads. Use AddVectoredExceptionHandler instead.
    _ex_handler = AddVectoredExceptionHandler(1, on_exception);
#else
  const int x = SA_RESTART | SA_ONSTACK;
  _old_handlers[SIGQUIT] = signal(SIGQUIT, on_signal_);
  _old_handlers[SIGABRT] = signal(SIGABRT, on_failure_, x);
  _old_handlers[SIGSEGV] = signal(SIGSEGV, on_failure_, x);
  _old_handlers[SIGFPE] = signal(SIGFPE, on_failure_, x);
  _old_handlers[SIGBUS] = signal(SIGBUS, on_failure_, x);
  _old_handlers[SIGILL] = signal(SIGILL, on_failure_, x);
  signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE
#endif
}

FailureHandler::~FailureHandler() {
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
#ifdef _WIN32
  if (_ex_handler) {
        RemoveVectoredExceptionHandler(_ex_handler);
        _ex_handler = NULL;
    }
#else
  signal(SIGSEGV, SIG_DFL);
  signal(SIGFPE, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  signal(SIGILL, SIG_DFL);
#endif
}

void FailureHandler::on_signal(int sig) {
  signal(sig, _old_handlers[sig]);
  raise(sig);
}

void FailureHandler::on_failure(int sig) {
  auto& g = global();
  switch (sig) {
    case SIGABRT:
      LOG(FATAL) << "SIGABRT: aborted\n";
      break;
#ifndef _WIN32
    case SIGSEGV:
      LOG(FATAL) << "SIGSEGV: segmentation fault\n";
      break;
    case SIGFPE:
      LOG(FATAL) << "SIGFPE: floating point exception\n";
      break;
    case SIGBUS:
      LOG(FATAL) << "SIGBUS: bus error\n";
      break;
    case SIGILL:
      LOG(FATAL) << "SIGILL: illegal instruction\n";
      break;
#endif
    default:
      LOG(FATAL) << "caught unexpected signal\n";
      break;
  }

  signal(sig, _old_handlers[sig]);
  raise(sig);
}



}  // namespace co

}  // namespace tit

