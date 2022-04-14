//
// Created by titto on 2022/4/10.
//

#include <iostream>

#include "log/logging.h"
#include "scheduler.h"
#include "channel.h"

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


  co::Channel<int> chan;
  chan << 10;
  LOG(INFO) << "channel write number: 10";
  auto* scheduler =
      static_cast<co::SchedulerImpl*>(schedulerManager.NextScheduler());

  scheduler->AddNewTask([=](){
    int recv;
    chan >> recv;
    LOG(INFO) << "revc: " << recv;
//    f();
  });

//  scheduler->AddNewTask([](){
//    f();
//  });
//
//  scheduler->AddNewTask([](){
//    f();
//  });

  LOG(INFO) << "g() end";
}


int main () {

  auto* scheduler =
      static_cast<co::SchedulerImpl*>(schedulerManager.NextScheduler());

//  std::function<void()> f_ = std::bind(&f);
  std::function<void()> g_ = std::bind(&g, 1);
//  scheduler->AddNewTask(f_);
  scheduler->AddNewTask(g_);

  char ch;
  std::cin >> ch;
}

