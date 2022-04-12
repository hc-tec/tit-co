//
// Created by titto on 2022/4/12.
//

#ifndef TIT_COROUTINE_TASK_MANAGER_H
#define TIT_COROUTINE_TASK_MANAGER_H

#include <vector>

#include "closure.h"
#include "base/mutex.h"

namespace tit {

namespace co {

struct Coroutine;

typedef base::MutexLock ThreadMutex;
typedef base::MutexLockGuard ThreadMutexGuard;

class TaskManager {
 public:

  typedef std::vector<Closure*> NewTaskList;
  typedef std::vector<Coroutine*> ReadyTaskList;

  // when task isn't yield, it's a new task
  void AddNewTasks(Closure* func);

  // when task is yield, and ready to resume, it's a ready task
  void AddReadyTasks(Coroutine* co);

  // get all tasks from task manager
  // input params will fill with tasks
  // so we needn't return value here
  void GetAllTasks(NewTaskList&, ReadyTaskList&);

 private:
  ThreadMutex mutex_;
  NewTaskList new_tasks_;
  ReadyTaskList ready_tasks_;
};


}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_TASK_MANAGER_H
