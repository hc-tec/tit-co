//
// Created by titto on 2022/4/10.
//

#include "scheduler.h"

#include <iostream>

using namespace tit;


void f () {
  for (int i = 0; i < 100; ++i) {
    std::cout <<  i << " " << std::endl;
  }
}


int main () {
  co::SchedulerManager schedulerManager(1, 1024*1024);
  auto* scheduler =
      static_cast<co::SchedulerImpl*>(schedulerManager.next_scheduler());
  std::function<void()> func = std::bind(&f);
  scheduler->add_new_task(&func);

  base::PlatformThread::Sleep(2000);
}

