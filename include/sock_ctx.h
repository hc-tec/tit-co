
#include <cstring>

#include "table.h"

namespace tit {
namespace co {

class SockCtx {
  public:
    SockCtx() = delete;

    // store id and scheduler id of the coroutine that performs read operation.
    void add_ev_read(int sched_id, int co_id) {
        _rev.s = sched_id;
        _rev.c = co_id;
    }

    // store id and scheduler id of the coroutine that performs write operation.
    void add_ev_write(int sched_id, int co_id) {
        _wev.s = sched_id;
        _wev.c = co_id;
    }

    void del_event() { memset(this, 0, sizeof(*this)); }
    void del_ev_read()  { _r64 = 0; }
    void del_ev_write() { _w64 = 0; }

    bool has_ev_read()  const { return _rev.c != 0; }
    bool has_ev_write() const { return _wev.c != 0; }

    bool has_ev_read(int sched_id) const {
        return _rev.s == sched_id && _rev.c != 0;
    }

    bool has_ev_write(int sched_id) const {
        return _wev.s == sched_id && _wev.c != 0;
    }

    bool has_event() const {
        return this->has_ev_read() || this->has_ev_write();
    }

    int32_t get_ev_read(int sched_id) const {
        return _rev.s == sched_id ? _rev.c : 0;
    }

    int32_t get_ev_write(int sched_id) const {
        return _wev.s == sched_id ? _wev.c : 0;
    }

  private:
    struct event_t {
      int32_t s; // scheduler id
      int32_t c; // coroutine id
    };
    union { event_t _rev; uint64_t _r64; };
    union { event_t _wev; uint64_t _w64; };
};


inline SockCtx& get_sock_ctx(size_t sock) {
    static co::table<SockCtx> k_sock_ctx_tb(14, 17);
    return k_sock_ctx_tb[sock];
}

} // co

}  // namespace tit