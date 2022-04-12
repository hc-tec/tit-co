# cocoyaxiIO调度

与IO操作最相关也最重要的是SchedulerImpl类下的Epoll\*成员。

```c++
class SchedulerImpl : public co::Scheduler {
  ...
  private:
    Epoll* _epoll;
    ...
}
```

看看Epoll类长啥样

```c++
class Epoll {
  public:
    Epoll(int sched_id);
    ~Epoll();
    
    // 管理 fd 事件
    bool add_ev_read(int fd, int32 co_id);
    bool add_ev_write(int fd, int32 co_id);
    void del_ev_read(int fd);
    void del_ev_write(int fd);
    void del_event(int fd);
    int wait(int ms);
    
    // 唤醒 epoll，有事件可能就绪了
    void signal(char c = 'x');
    
    const epoll_event& operator[](int i)   const { return _ev[i]; }
    // ev 绑定的 fd
    int user_data(const epoll_event& ev)         { return ev.data.fd; }
    // 是否为唤醒管道 fd    
    bool is_ev_pipe(const epoll_event& ev) const { return ev.data.fd == _pipe_fds[0]; }
    void handle_ev_pipe();
    void close();

  private:
    int _ep; // epoll fd
    int _pipe_fds[2]; // wakeUp pipes 唤醒管道
    int _sched_id;
    epoll_event* _ev;
    bool _signaled;
};    
```

从Epoll的成员函数可以发现，它承担着管理fd读写事件的功能。

Epoll通过管道实现唤醒，其实就是向写端发送一个字节数据，由于读端正被epoll\_wait监听，这样一来epoll\_wait就会返回，达到唤醒epoll的目的。

```c++
// 唤醒 epoll，有事件可能就绪了
void signal(char c = 'x') {
  if (atomic_compare_swap(&_signaled, false, true) == false) {
    const int r = (int) CO_RAW_API(write)(_pipe_fds[1], &c, 1);
    ELOG_IF(r != 1) << "pipe write error..";
  }
}
int _pipe_fds[2]; // wakeUp pipes 唤醒管道
```

继续看Epoll如何实现对 fd 读写事件的管理的。

```c++
bool Epoll::add_ev_read(int fd, int32 co_id) {
    if (fd < 0) return false;
    auto& ctx = co::get_sock_ctx(fd);
    if (ctx.has_ev_read()) return true; // already exists
    
    // 正要添加的是读事件
    // 可能已经添加了写事件，干脆就一起处理
    const bool has_ev_write = ctx.has_ev_write(_sched_id);
    epoll_event ev;
    ev.events = has_ev_write ? (EPOLLIN | EPOLLOUT | EPOLLET) : (EPOLLIN | EPOLLET);
    ev.data.fd = fd;
    
    const int r = epoll_ctl(_ep, has_ev_write ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &ev);
    if (r == 0) {
        // 添加成功，修改上下文，将调度器id和协程id添加到上下文
        ctx.add_ev_read(_sched_id, co_id);
        return true;
    } else {
        ELOG << "epoll add ev read error: " << co::strerror() << ", fd: " << fd << ", co: " << co_id;
        return false;
    }
}
```

对上面第3行代码继续细分

```c++
auto& ctx = co::get_sock_ctx(fd);


inline SockCtx& get_sock_ctx(size_t sock) {
    // k_sock_ctx_tb 是一张全局表，包含了 fd 的上下文(SockCtx)
    static auto& k_sock_ctx_tb = *co::new_fixed<co::table<SockCtx>>(14, 17);
    return k_sock_ctx_tb[sock];
}

class SockCtx {
  public:
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
    // 只有在调度器 id 匹配时才能读写
    int32 get_ev_read(int sched_id) const {
        return _rev.s == sched_id ? _rev.c : 0;
    }
    int32 get_ev_write(int sched_id) const {
        return _wev.s == sched_id ? _wev.c : 0;
    }
  ...
  private:
    // 通过 64 位数字来保存两个id
    // 前 32 位是调度器 id
    // 后 32 位是协程 id
    struct event_t {
        int32 s; // scheduler id
        int32 c; // coroutine id
    };
    // 对读写事件进行了区别
    // 因为一个协程在读的同时，另一个协程在写
    // 使用 union 可以方便清空，见 del_ev_read 方法
    union { event_t _rev; uint64 _r64; };
    union { event_t _wev; uint64 _w64; };
}

```

Epoll 除了fd事件管理和唤醒管道值得注意外，还有最重要的一处，系统函数hook。

```c++
class Epoll {
  public:
    int wait(int ms) {
      // epoll_wait 已被 hook
      return CO_RAW_API(epoll_wait)(_ep, _ev, 1024, ms);
    }
}


#define CO_RAW_API(x)         co_raw_##x
// 宏中的 ## 是拼接符
// CO_RAW_API(x) -> co_raw_epoll_wait

#define _CO_DEC_RAW_API(x)    extern x##_fp_t CO_RAW_API(x)
_CO_DEC_RAW_API(epoll_wait);
// _CO_DEC_RAW_API(epoll_wait) -> extern epoll_wait_fp_t co_raw_epoll_wait

typedef int (*epoll_wait_fp_t)(int, struct epoll_event*, int, int);
// co_raw_epoll_wait 实际上是函数指针

hook_api(epoll_wait);
// 实际的 epoll_wait 实现
int epoll_wait(int epfd, struct epoll_event* events, int n, int ms) {
    hook_api(epoll_wait);
    HOOKLOG << "hook epoll_wait, fd: " << epfd << ", n: " << n << ", ms: " << ms;
    if (!co::gSched || epfd < 0 || ms == 0) return CO_RAW_API(epoll_wait)(epfd, events, n, ms);

    co::IoEvent ev(epfd, co::ev_read);
    if (!ev.wait(ms)) return 0; // timeout
    return CO_RAW_API(epoll_wait)(epfd, events, n, 0);
}

#define hook_api(f) \
   if (!CO_RAW_API(f)) atomic_set(&CO_RAW_API(f), dlsym(RTLD_NEXT, #f))
// 这里最终得到实际的 epoll_wait 实现
```

上面28行代码处新建了一个co::IoEvent，我们对28-29行进行分析，因为这是协程如何阻塞的实现关键

```c++
enum io_event_t {
    ev_read = 1,
    ev_write = 2,
};
/**
 * IoEvent is for waiting an IO event on a socket 
 *   - It MUST be used in a coroutine. 
 *   - The socket MUST be non-blocking on Linux & Mac. 
 */
class IoEvent {
  public:
    IoEvent(sock_t fd, io_event_t ev)
        : _fd(fd), _ev(ev), _has_ev(false) {
    }
    // 析构时，需要把事件从调度器中删除
    ~IoEvent();
    // wait 中进行了协程切换的实现
    bool wait(uint32 ms=(uint32)-1);
  private:
    sock_t _fd;
    io_event_t _ev;
    bool _has_ev;
    DISALLOW_COPY_AND_ASSIGN(IoEvent);
};
```

```c++
// thread local 的调度器，每个线程有一个独立的调度器
__thread SchedulerImpl* gSched;

// 删除当前 fd 的 io 事件
IoEvent::~IoEvent() {
    if (_has_ev) gSched->del_io_event(_fd, _ev);
}

bool IoEvent::wait(uint32 ms) {
    auto s = gSched;
    if (!_has_ev) {
        _has_ev = s->add_io_event(_fd, _ev);
        if (!_has_ev) return false;
    }
    
    // 如果有超时时间
    if (ms != (uint32)-1) {
        s->add_timer(ms);
        // 切换协程
        s->yield();
        if (!s->timeout()) {
            return true;
        } else {
            errno = ETIMEDOUT;
            return false;
        }
    } else {
        // 切换协程
        s->yield();
        return true;
    }
}
```

cocoyaxi总共对这些系统函数进行了hook

```c++
_CO_DEF_RAW_API(socket);
_CO_DEF_RAW_API(socketpair);
_CO_DEF_RAW_API(pipe);
_CO_DEF_RAW_API(pipe2);
_CO_DEF_RAW_API(fcntl);
_CO_DEF_RAW_API(ioctl);
_CO_DEF_RAW_API(dup);
_CO_DEF_RAW_API(dup2);
_CO_DEF_RAW_API(dup3);
_CO_DEF_RAW_API(setsockopt);
_CO_DEF_RAW_API(close);
_CO_DEF_RAW_API(shutdown);
_CO_DEF_RAW_API(connect);
_CO_DEF_RAW_API(accept);
_CO_DEF_RAW_API(read);
_CO_DEF_RAW_API(readv);
_CO_DEF_RAW_API(recv);
_CO_DEF_RAW_API(recvfrom);
_CO_DEF_RAW_API(recvmsg);
_CO_DEF_RAW_API(write);
_CO_DEF_RAW_API(writev);
_CO_DEF_RAW_API(send);
_CO_DEF_RAW_API(sendto);
_CO_DEF_RAW_API(sendmsg);
_CO_DEF_RAW_API(poll);
_CO_DEF_RAW_API(select);
_CO_DEF_RAW_API(sleep);
_CO_DEF_RAW_API(usleep);
_CO_DEF_RAW_API(nanosleep);
_CO_DEF_RAW_API(gethostbyname);
_CO_DEF_RAW_API(gethostbyaddr);

// Linux
_CO_DEF_RAW_API(epoll_wait);
_CO_DEF_RAW_API(accept4);
_CO_DEF_RAW_API(gethostbyname2);
_CO_DEF_RAW_API(gethostbyname_r);
_CO_DEF_RAW_API(gethostbyname2_r);
_CO_DEF_RAW_API(gethostbyaddr_r);
```

hook的最终目的是让这些函数**协程化改造**。一般都要求 fd 为非阻塞

比如在没有客户端连接时，原本的 accept 的返回值为-1，fd 可能在epoll循环中打转，此时代码编写变成了异步回调模式。

协程化改造之后，accept在没有客户端连接时返回-1，这时会将令当前协程阻塞，转而去执行其他的协程；当有新的客户端连接来时，accept恢复执行，返回接受的 fd，此时代码编写可以完全同步化，而不是异步回调模式。



再看事件循环

```c++
void SchedulerImpl::loop() {
    gSched = this;
    co::vector<Closure*> new_tasks;
    co::vector<Coroutine*> ready_tasks;

    while (!_stop) {
        int n = _epoll->wait(_wait_ms);
        if (_stop) break;

        if (unlikely(n == -1)) {
            if (errno != EINTR) {
                ELOG << "epoll wait error: " << co::strerror();
            }
            continue;
        }

        for (int i = 0; i < n; ++i) {
            auto& ev = (*_epoll)[i];
            if (_epoll->is_ev_pipe(ev)) {
                _epoll->handle_ev_pipe();
                continue;
            }

            ...
            int32 rco = 0, wco = 0;
            // SockCtx 上下文
            auto& ctx = co::get_sock_ctx(_epoll->user_data(ev));
            if ((ev.events & EPOLLIN)  || !(ev.events & EPOLLOUT)) rco = ctx.get_ev_read(this->id());
            if ((ev.events & EPOLLOUT) || !(ev.events & EPOLLIN))  wco = ctx.get_ev_write(this->id());
            if (rco) this->resume(_co_pool[rco]);
            if (wco) this->resume(_co_pool[wco]);
            ...
            
        }

        CO_DBG_LOG << "> check tasks ready to resume..";
        do {
            _task_mgr.get_all_tasks(new_tasks, ready_tasks);

            if (!new_tasks.empty()) {
                CO_DBG_LOG << ">> resume new tasks, num: " << new_tasks.size();
                for (size_t i = 0; i < new_tasks.size(); ++i) {
                    this->resume(this->new_coroutine(new_tasks[i]));
                }
                new_tasks.clear();
            }

            if (!ready_tasks.empty()) {
                CO_DBG_LOG << ">> resume ready tasks, num: " << ready_tasks.size();
                for (size_t i = 0; i < ready_tasks.size(); ++i) {
                    this->resume(ready_tasks[i]);
                }
                ready_tasks.clear();
            }
        } while (0);

        CO_DBG_LOG << "> check timedout tasks..";
        do {
            _wait_ms = _timer_mgr.check_timeout(ready_tasks);

            if (!ready_tasks.empty()) {
                CO_DBG_LOG << ">> resume timedout tasks, num: " << ready_tasks.size();
                _timeout = true;
                for (size_t i = 0; i < ready_tasks.size(); ++i) {
                    this->resume(ready_tasks[i]);
                }
                _timeout = false;
                ready_tasks.clear();
            }
        } while (0);

        if (_running) _running = 0;
    }

    _ev.signal();
}
```

