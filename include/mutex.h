

#include <queue>

#include "base/mutex.h"
#include "base/noncopyable.h"
#include "def.h"
#include "atomic.h"
#include "scheduler.h"

namespace tit {

namespace co {

struct Coroutine;

typedef base::MutexLock ThreadMutexType;
typedef base::MutexLockGuard ThreadMutexLockGuardType;

// coroutine mutex
// only use in coroutine !!!
class Mutex {
 public:
  Mutex() : lock_(false) {}
  ~Mutex() = default;

  void Lock();

  void UnLock();

  bool TryLock();

 private:
  ThreadMutexType mutex_;
  std::queue<Coroutine*> wait_queue_;
  bool lock_;
};


}  // namespace co

}  //  namespace tit