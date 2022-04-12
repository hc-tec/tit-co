//
// Created by titto on 2022/4/12.
//

#include "task_manager.h"
#include "coroutine.h"
#include "closure.h"

namespace tit {

namespace co {

void TaskManager::AddNewTasks(Closure func) {
  ThreadMutexGuard guard(mutex_);
  new_tasks_.push_back(func);
}

void TaskManager::AddReadyTasks(Coroutine* co) {
  ThreadMutexGuard guard(mutex_);
  ready_tasks_.push_back(co);
}

void TaskManager::GetAllTasks(TaskManager::NewTaskList& new_tasks,
                              TaskManager::ReadyTaskList& ready_tasks) {
  ThreadMutexGuard guard(mutex_);
  if (!new_tasks_.empty()) {
    std::swap(new_tasks, new_tasks_);
  }
  if (!ready_tasks_.empty()) {
    std::swap(ready_tasks, ready_tasks_);
  }
}

}  // namespace co

}  // namespace tit
