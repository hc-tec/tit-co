
# Channel实现
### 官方文档描述

<https://idealvin.github.io/cn/co/coroutine/#channelcochan>

`co::Chan` 是一个模板类，它类似于 golang 中的 channel，用于在协程之间传递数据。

```cpp
template <typename T> class Chan;

```

*   `co::Chan` 内部基于内存拷贝实现，模板参数 T 可以是内置类型、指针类型，或者**拷贝操作具有简单的内存拷贝语义的结构体类型**。简而言之，T 必须满足下述条件：对于 T 类型的两个变量或对象 a 与 b, a = b 等价于 memcpy(&a, &b, sizeof(T))。

*   像 `std::string` 或 STL 中的容器类型，拷贝操作不是简单的内存拷贝，因此不能直接在 channel 中传递。

#### 代码示例

```cpp
#include "co/co.h"

void f() {
    co::Chan<int> ch;
    go([ch]() { ch << 7; });
    int v = 0;
    ch >> v;
    LOG << "v: " << v;
}

void g() {
    co::Chan<int> ch(32, 500);
    go([ch]() {
        ch << 7;
        if (co::timeout()) LOG << "write to channel timeout..";
    });

    int v = 0;
    ch >> v;
    if (!co::timeout()) LOG << "v: " << v;
}

DEF_main(argc, argv) {
    f();
    g();
    return 0;
}
```

上述代码中的 channel 对象在栈上，而 CO 采用的是共享栈实现方式，一个协程栈上的数据可能被其他协程覆盖，**协程间一般不能直接通过栈上的数据通信**，因此代码中的 lambda 采用了**按值捕获**的方式，将 channel 拷贝了一份，传递到新建的协程中。channel 的拷贝操作只是将内部引用计数加 1，几乎不会对性能造成影响。



### 前言

Channel是一种常见的生产者-消费者模型应用。常用于信息的传递。

cocoyaxi库中Channel的实现可以说很有特色，从上述官方文档中的代码示例中可以看到，channel还提供了超时功能，给开发者更好的灵活性。

### 深入Channel

```cpp
// chan.h
template <typename T>
class Chan {
  public:
    /**
     * @param cap  max capacity of the queue, 1 by default.
     * @param ms   default timeout in milliseconds, -1 by default.
     */
    explicit Chan(uint32 cap=1, uint32 ms=(uint32)-1)
        : _p(cap * sizeof(T), sizeof(T), ms) {
    }

    ~Chan() = default;

    Chan(Chan&& c) : _p(std::move(c._p)) {}

    Chan(const Chan& c) : _p(c._p) {}

    void operator=(const Chan&) = delete;

    void operator<<(const T& x) const {
        _p.write(&x);
    }

    void operator>>(T& x) const {
        _p.read(&x);
    }

  private:
    xx::Pipe _p;
};
```

co::Chan的定义见chan.h，可以看出，channel具体实现交给了xx::Pipe。

不过这里毕竟是提供给开发者最外层的接口，代码值得看看。

继续正文，具体的实现链条是这种的：

co::Chan  →  xx::Pipe  →  xx::PipeImpl

接下来直接看xx::PipeImpl的代码

```cpp
class PipeImpl {
  public:
    PipeImpl(uint32 buf_size, uint32 blk_size, uint32 ms)
        : _buf_size(buf_size), _blk_size(blk_size), 
          _rx(0), _wx(0), _ms(ms), _full(false) {
        _buf = (char*) malloc(_buf_size);
    }

    void read(void* p);
    void write(const void* p);
    
    ...

  private:
    ::Mutex _m;
    std::deque<waitx*> _wq;
    char* _buf;       // buffer
    uint32 _buf_size; // buffer size
    uint32 _blk_size; // block size
    ...
    uint32 _ms;       // timeout in milliseconds
    bool _full;       // 0: not full, 1: full
};
```

先从构造函数中看成员变量的作用，简单分析一下调用链

```cpp
    /**
     * @param cap  max capacity of the queue, 1 by default.
     * @param ms   default timeout in milliseconds, -1 by default.
     */
    explicit Chan(uint32 cap=1, uint32 ms=(uint32)-1)
        : _p(cap * sizeof(T), sizeof(T), ms) {
    }
    
    PipeImpl(uint32 buf_size, uint32 blk_size, uint32 ms) {
      ...
    }

```

不难发现其中的对应关系：

buf\_size  →  cap \* sizeof(T)

blk\_size  →  sizeof(T)

ms  →  ms

sizeof(T)，其中T是模板参数，即此次信息传递的类型，sizeof(T)就能求得此类型的大小，blk\_size刚好是sizeof(T)。我们称其为块大小（block size）。

cap为容量，也就是此次消息传递最大数量，所以缓冲区buf\_分配了刚好能装下cap个数据的空间。

ms是超时时间，以毫秒为单位。

接下来看看其他的内容

```cpp
class PipeImpl {
  ...
  struct waitx {
    co::Coroutine* co;
    union {
        uint8 state;
        void* dummy;
    };
    void* buf;
  ...
};

```

waitx结构体可以理解为等待上下文，它用在两个地方：

1.  外部需要读数据时（对变量来说就是写操作，直接操作指针），也就是需要读缓冲区时，如果缓冲区是空的，没有数据可读，这时候需要挂起当前协程。

2.  外部需要写数据时（对变量来说就是读操作），也就是写缓冲区时，如果缓冲区是满的，无法写入数据，这时候需要挂起当前协程。

waitx有四种状态，不必多言

```cpp
enum co_state_t : uint8 {
    st_init = 0,     // initial state
    st_wait = 1,     // wait for an event
    st_ready = 2,    // ready to resume
    st_timeout = 4,  // timeout
};
```

还剩下一个void\* buf 成员，还需要综合一下才能知道它的用处。

```cpp
  // buf 其实就是需要读写的变量的指针
  waitx* create_waitx(co::Coroutine* co, void* buf) {
      waitx* w;
      // 判断 buf 是否在栈上
      const bool on_stack = gSched->on_stack(buf);
      if (on_stack) {
          // 协程切换出去可能会影响到变量的值，见后文分析
          // 变量每次读一个块大小，所以只需多分配一个 _blk_size
          w = (waitx*) malloc(sizeof(waitx) + _blk_size);
          // w->buf 指向 waitx 结构体的末尾
          w->buf = (char*)w + sizeof(waitx);
      } else {
          // 变量没有定义在当前的协程栈上
          // 协程切换出去也影响不到变量的值
          // w->buf 仍然使用现有变量指针
          w = (waitx*) malloc(sizeof(waitx));
          w->buf = buf;
      }
      w->co = co;
      w->state = st_init;
      return w;
  }
```

为什么需要根据变量是否在协程栈上来对buf进行两种不同的操作呢？

如果是协程内创建的变量从channel中读数据，（正在运行的协程使用共享栈），当前协程挂起后，共享栈中的数据可能会被其他协程占用，协程内创建的变量就会转移到私有栈中，注意，向变量指针写入读取到的值这一过程可能并不发生在变量所在的协程中，这时如果修改了变量指针，其实修改的是共享栈上的内存值，而不是挂起的协程私有栈上的变量，这就会导致异常出现。

所以，需要分配一块新的空间来暂时保存读写的值。这可以认为是对栈指针变量的一种保护。



> 举个通俗的例子，足球场是一个公共场所，有许多足球队等着去里面踢球。在某一时刻，A足球队正在准备踢球，张三缺了双好球鞋，A球队中的前锋张三打了朋友的电话，让朋友帮他把球鞋送来，不过朋友才刚出门，送来的没那么快。不料这时球队经理找他们球队开会，说是要商量晚上是吃葱爆海参还是煮海参汤喝。球队成员立刻兴高采烈地离开球场前往开会场所。去了没多久，朋友送来了鞋，张三告诉他子按在站在球场门口，现在站在球场确实有一个人，不过他并不是张三。朋友就想把鞋子委托给附近的人，让他们帮忙把球鞋交给张三。可是，该找谁好呢？找其他球队的人?没准他们是外地来的球队，踢完一局就永远不会再来了，况且他们也不认得张三。那就交给附近认得张三，又经常来训练的老年人吧，朋友委托给了一个老人，之后就回去了。张三一行人商量好了要吃葱爆海参十分高兴，回到了球场继续踢球。不过张三还少了双好球鞋，受委托的老人挺仗义，看到张三来了立马把球鞋交给了他。于是张三又能够愉快地在脑子里边想着海参边踢球了。

> 球场相当于共享栈，球队相当于协程，球队可以有自己的休息场地（私有栈，中断时用于保存协程栈）。不过，要想踢球（运行）必须在球场（共享栈）上才行。
> A球队的张三相当于协程栈上的一个变量，球队走了，他也会跟着走。
> 张三需要双好球鞋，相当于变量想从Channel中读数据。朋友相当于向Channel写数据的协程。
> A球队要开会相当于协程中断，此时球场可能被别的球队占用了。
> **不过送鞋的朋友可不管场上踢球的是不是张三的球队**。
> 他到了，要么把鞋交给张三，要么委托给别人。
> 可是张三已经跟着球队离开了。虽然张三告诉朋友，他在球场门口等他，可是现在就算球场真的有人，那也不是真的张三，可能变成了B球队的李四，不可能把鞋交给他吧（毕竟不是他的鞋，他也很可能不认得张三）。
> 因此朋友需要找到一个待的时间足够长，有认识张三的人，可以是经常待在球场的老人，也可以是花钱临时委托的小孩子。
> 总之，当球队再次回来踢球的时候，委托人就得把球鞋交给张三。

总而言之，其实问题所在是因为张三只给了朋友**当时他所在的位置**。他跟着球队离开之后跑哪去了，朋友并不清楚。认识到这点问题所在，理解起来就轻松了。

还剩下最后一些成员变量需要介绍下：

```cpp
::Mutex _m;  // 互斥锁，管理临界区
std::deque<waitx*> _wq;  // 等待队列
char* _buf;       // buffer 缓冲区
uint32 _buf_size; // buffer size  缓冲区大小
uint32 _blk_size; // block size [一块数据]的大小
uint32 _rx;       // read pos  读指针 读偏移
uint32 _wx;       // write pos 写指针 写偏移
uint32 _ms;       // timeout in milliseconds  超时时间
bool _full;       // 0: not full, 1: full 缓冲区是否已满
```

类介绍完了，后面的读写方法才是重头戏。

#### read方法

read是指从缓冲区读取数据，写入到传进来的指针p中。

```cpp
void PipeImpl::read(void* p) {
    // thread local 全局调度器
    auto s = gSched;
    CHECK(s) << "must be called in coroutine..";

    _m.lock();
    // 读写指针不相等，说明缓冲区不是空的，也不是满的
    if (_rx != _wx) { /* buffer is neither empty nor full */
        assert(!_full);
        assert(_wq.empty());
        // 向指针 p 写入数据
        // _buf指向缓冲区起始地址，_buf+_rx是指读指针当前位置
        // 这句代码的意思是
        // 从读指针开始，向后读取 _blk_size 长度的数据，复制到 p 指针指向的地址中
        memcpy(p, _buf + _rx, _blk_size);
        // 读指针向后移动
        _rx += _blk_size;
        // 如果移动到头了，重新归位
        // 缓冲期可以看成是环形队列
        if (_rx == _buf_size) _rx = 0;
        _m.unlock();

    } else {
        // 缓冲区为空，读不了数据了，协程需要挂起
        if (!_full) { /* buffer is empty */
            // 当前协程
            auto co = s->running();
            // 创建等待上下文
            waitx* w = this->create_waitx(co, p);
            // 加入等待队列
            _wq.push_back(w);
            _m.unlock();
            
            // 协程调度器更新
            if (co->s != s) co->s = s;
            co->waitx = w;
            // 启动超时功能
            if (_ms != (uint32)-1) s->add_timer(_ms);
            // 挂起协程
            s->yield();
            // 运行到这里，说明协程已恢复
            // 判断是否超时
            if (!s->timeout()) {
                // 从前文 create_waitx 的分析可以发现
                // 当 p 在协程栈中时 w->buf 不会等于 p
                // 因为已经分配了一块新空间替代 p
                // 不过该是人家的东西还是人家的
                // 这里要还回给 p
                if (w->buf != p) memcpy(p, w->buf, _blk_size);
                ::free(w);
            }

            co->waitx = 0;

        } else { /* buffer is full */
            // 缓冲区已经满了，说明生产速度较快，消费速度较慢
            // 此时等待队列中是不是只能有想写但又写不了的协程
            // 不可能出现想读的协程
            memcpy(p, _buf + _rx, _blk_size);
            _rx += _blk_size;
            if (_rx == _buf_size) _rx = 0;

            while (!_wq.empty()) {
                waitx* w = _wq.front(); // wait for write
                _wq.pop_front();

                if (atomic_compare_swap(&w->state, st_init, st_ready) == st_init) {
                    // 取出的是写协程等待上下文，而非读协程等待上下文
                    // 想想这是为什么
                    // 因此这里是写操作
                    memcpy(_buf + _wx, w->buf, _blk_size);
                    _wx += _blk_size;
                    if (_wx == _buf_size) _wx = 0;
                    _m.unlock();
                    // 调度写协程
                    ((co::SchedulerImpl*) w->co->s)->add_ready_task(w->co);
                    return;

                } else { /* timeout */
                    ::free(w);
                }
            }

            _full = false;
            _m.unlock();
        }
    }
}
```

#### write方法

write是指从指针p中读数据，写入到缓冲区中。

p指针只读不写，因此write参数void\* p加了个const修饰符。

```cpp
void PipeImpl::write(const void* p) {
    // thread local 全局调度器
    auto s = gSched;
    CHECK(s) << "must be called in coroutine..";

    _m.lock();
    if (_rx != _wx) { /* buffer is neither empty nor full */
        assert(!_full);
        assert(_wq.empty());
        // 将p指针的数据复制到写指针处
        memcpy(_buf + _wx, p, _blk_size);
        // 写指针右移
        _wx += _blk_size;
        // 写到头了，归位
        if (_wx == _buf_size) _wx = 0;
        // 读写指针相遇，说明东西满了
        // 缓冲期可以看成是环形队列
        // 读写指针相遇，说明缓冲区要么空，要么满
        // 而目前正在向缓冲区写数据
        // 所以只可能是满的，不可能空
        if (_rx == _wx) _full = true;
        _m.unlock();

    } else {
        if (!_full) { /* buffer is empty */
            // 如果缓冲区是空的，说明消费速度较快，生产速度较慢
            // 如果等待队列有数据，那一定是想读而读不了的协程
            // 不可能出现想写的协程
            while (!_wq.empty()) {
                // 从缓冲期中取出一个等待上下文，其中包含了等待协程
                // 协程只能是读协程
                waitx* w = _wq.front(); // wait for read
                _wq.pop_front();

                if (atomic_compare_swap(&w->state, st_init, st_ready) == st_init) {
                    _m.unlock();
                    // 将当前指针数据写入到 w->buf 
                    // 对w->buf 来说就是“读“到了数据
                    memcpy(w->buf, p, _blk_size);
                    // 协程拿到了数据，调度协程
                    ((co::SchedulerImpl*) w->co->s)->add_ready_task(w->co);
                    // 当前已有内容已经读完了，没东西可读，退出函数
                    // 并不代表等待队列没有等待协程
                    return;
                } else { /* timeout */
                    ::free(w);
                }
            }
            // 运行到这里，说明等待队列要么没东西，要么是等待上下文已超时
            // 总而言之，等待队列中没有读成功
            // 现在就可以把指针数据读取后写入到缓冲区中
            memcpy(_buf + _wx, p, _blk_size);
            _wx += _blk_size;
            if (_wx == _buf_size) _wx = 0;
            if (_rx == _wx) _full = true;
            _m.unlock();

        } else { /* buffer is full */
            // 缓冲区已满，不能再写了，需要挂起协程
            auto co = s->running();
            waitx* w = this->create_waitx(co, (void*)p);
            // 变量是在栈中分配的，因此w->buf指向的并不是原变量指针
            // 而是跟在等待上下文(waitx)后面的一块新的空间
            // 因此需要重新保存变量的值
            if (w->buf != p) memcpy(w->buf, p, _blk_size);
            _wq.push_back(w);
            _m.unlock();

            if (co->s != s) co->s = s;
            co->waitx = w;

            if (_ms != (uint32)-1) s->add_timer(_ms);
            // 挂起协程
            s->yield();
            // 运行到这里，说明协程已恢复
            // 判断是否超时
            if (!s->timeout()) ::free(w);
            co->waitx = 0;
        }
    }
}
```

