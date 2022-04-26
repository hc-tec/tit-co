//
// Created by titto on 2022/4/10.
//

#include "scheduler.h"

#include <iostream>

#include "channel.h"
#include "interfaces/connection_interface.h"
#include "log/logging.h"
#include "net/address.cc"
#include "net/socket.cc"
#include "net/tcp.cc"
#include "sock.cc"

using namespace tit;



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


  co::Channel<int> chan(1, 1);
//  chan << 10;
  LOG(INFO) << "channel write number: 10";
  auto* scheduler =
      static_cast<co::SchedulerImpl*>(co::schedulerManager().NextScheduler());

  scheduler->AddNewTask([=](){
    int recv;
    chan >> recv;
    if (co::timeout()) {
      LOG(ERROR) << "timeout!";
    }

//    LOG(INFO) << "revc: " << recv;
//    f();
  });
  int recv;
//  chan >> recv;
//  chan >> recv;
  LOG(INFO) << "revc: " << recv;
//  chan << 11;
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
      static_cast<co::SchedulerImpl*>(co::schedulerManager().NextScheduler());

//  std::function<void()> f_ = std::bind(&f);
  std::function<void()> g_ = std::bind(&g, 1);
//  scheduler->AddNewTask(f_);
  scheduler->AddNewTask(g_);

  char ch;
  std::cin >> ch;
}

