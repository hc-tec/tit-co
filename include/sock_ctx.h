//
// Created by titto on 2022/4/13.
//

#ifndef TIT_COROUTINE_SOCK_CTX_H
#define TIT_COROUTINE_SOCK_CTX_H

#include <map>

#include "def.h"
#include "base/singleton.h"

namespace tit {

namespace co {

class SockCtx {
 public:
  SockCtx() = delete;
  // The minimum unit of io schedule is coroutine.
  // A socket can be keep in two different coroutines,
  // one is reading the socket, meanwhile,
  // another one is writing the socket,
  // so it's necessary to two different context.
  // In socket context, we use high 32 bit as scheduler ID
  // and low 32 bit as coroutine ID
  void AddEvRead(uint32 sched_id, uint32 co_id);
  void AddEvWrite(uint32 sched_id, uint32 co_id);
  void DelEvRead();
  void DelEvWrite();
  void DelEvent();

  bool has_event() const;
  bool has_ev_read() const;
  bool has_ev_read(uint32 sched_id) const;
  bool has_ev_write() const;
  bool has_ev_write(uint32 sched_id) const;

  uint32 get_read_co_id(uint32 sched_id) const;
  uint32 get_write_co_id(uint32 sched_id) const;

 private:

  typedef struct {
    uint32 co_id_;
    uint32 sched_id_;
  } event_t;

  union {
    event_t read_ev_;
    uint64 read_clear_;  // use for quick clear
  };
  union {
    event_t write_ev_;
    uint64 write_clear_;  // use for quick clear
  };
};


typedef std::map<int, SockCtx> SockCtxMap;
typedef base::Singleton<SockCtxMap> SockCtxSingleton;

//#define SockCtxInstance SockCtxSingleton::instance();

SockCtx& get_sock_ctx(int fd);

}  // namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_SOCK_CTX_H
