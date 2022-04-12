//
// Created by titto on 2022/4/10.
//

#include <iostream>


#include "log/logging.h"

#include "scheduler.h"
#include "coroutine.cc"
#include "t_time.cc"

#include "mutex.cc"

#include "sock.cc"


using namespace tit;

static co::SchedulerManager schedulerManager(1, 1024*1024);

void f () {
  LOG(INFO) << "f() begin";
  for (int i = 0; i < 3; ++i) {
    LOG(INFO) <<  i << " ";
  }
  LOG(INFO) << "f() end";
}

void g (int a) {
  LOG(INFO) << "g() begin";
  LOG(INFO) << a;
  LOG(INFO) << "g() end";

  auto* scheduler =
      static_cast<co::SchedulerImpl*>(schedulerManager.next_scheduler());

  scheduler->add_new_task([](){
    f();
  });

  scheduler->add_new_task([](){
    f();
  });

  scheduler->add_new_task([](){
    f();
  });
}


int main () {

  auto* scheduler =
      static_cast<co::SchedulerImpl*>(schedulerManager.next_scheduler());
  std::function<void()> f_ = std::bind(&f);
  std::function<void()> g_ = std::bind(&g, 1);
  scheduler->add_new_task(f_);
  scheduler->add_new_task(g_);

  char ch;
  std::cin >> ch;
}

