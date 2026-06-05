# MiniHttp

一个基于 Linux epoll 的 C++17 高性能 HTTP 服务器，采用 **主从 Reactor（Master-Slave Reactor）** 架构。

---

## 架构

```
┌─ main() ──────────────────────────────────────────────────┐
│                                                           │
│   HttpServer(port=8080, io_thread=2, worker=4)            │
│                                                           │
│   ┌── Master Reactor (main 线程) ──────────────────────┐  │
│   │  epoll: 仅监听 server_fd + signalfd                │  │
│   │  职责: accept 新连接 + 信号处理                     │  │
│   └─────────────────────┬───────────────────────────────┘  │
│                         │ round-robin 分发                 │
│            ┌────────────┴────────────┐                    │
│            ▼                         ▼                     │
│   ┌── Slave Reactor 1 ────┐ ┌── Slave Reactor 2 ────┐    │
│   │ 线程1: epoll_wait     │ │ 线程2: epoll_wait     │    │
│   │ read → parse → 回调   │ │ read → parse → 回调   │    │
│   │ Connection 池          │ │ Connection 池          │    │
│   └────────┬──────────────┘ └────────┬──────────────┘    │
│            │                          │                    │
│            └────────────┬─────────────┘                    │
│                         ▼                                  │
│   ┌── ThreadPool (4 worker 线程) ──────────────────────┐  │
│   │  CPU 密集计算 / 阻塞操作                            │  │
│   └────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────┘
```

**线程模型（共 7 线程）**：

| 角色 | 数量 | 职责 |
|------|------|------|
| Master Reactor | 1（main 线程） | accept + 信号处理 |
| Slave Reactor | N（默认 2） | I/O 读写 + HTTP 解析 + 回调 |
| Worker 线程 | M（默认 4） | CPU 密集计算 |

---

## 特性

- **主从 Reactor 架构** — 1 Master accept + N Slave I/O，充分利用多核
- **跨线程唤醒** — eventfd 实现线程间高效通知
- **HTTP/1.1 解析** — 有限状态机解析 Method / URI / Headers / Body
- **参数化路由** — 支持 `GET /user/:id/post/:pid` 动态路径匹配
- **Keep-Alive** — 支持长连接复用
- **线程池** — 异步处理耗时业务，不阻塞 I/O 线程
- **生命周期安全** — `shared_ptr<bool>` token 防止异步回调悬挂指针
- **RAII 资源管理** — Socket move-only 封装，自动管理 fd
- **优雅关闭** — signalfd + 逐层停止（Master → Slave → ThreadPool）
- **缓冲区安全** — token 上限 8KB，body 上限 1MB，连接缓冲上限 64KB

---

## 快速开始

### 依赖

- Linux（需要 epoll、eventfd、signalfd 支持）
- CMake ≥ 3.16
- 支持 C++17 的编译器（GCC 8+ / Clang 10+）

### 构建

```bash
mkdir build && cd build
cmake ..
make -j
```

### 运行

```bash
./minihttp_server
```

默认监听 `http://localhost:8080`。

---

## 路由示例

| 方法 | 路径 | 响应 |
|------|------|------|
| GET | `/` | `Hello World!` |
| GET | `/about` | `This is my server` |
| GET | `/user/:id` | `User: {id}` |
| GET | `/user/:id/post/:pid` | `User: {id}, Post: {pid}` |
| GET | `/slow` | 线程池异步计算的 URI 信息 |

---

## 核心模块

| 模块 | 文件 | 职责 |
|------|------|------|
| **Channel** | `core/channel.h/cpp` | I/O 事件通道，fd + 事件类型 + 回调 |
| **EventLoop** | `core/eventloop.h/cpp` | 事件循环基类，epoll 封装 + 跨线程任务队列 |
| **SlaveEventLoop** | `core/slaveeventloop.h/cpp` | 从 Reactor，继承 EventLoop + 连接管理 |
| **EventLoopGroup** | `core/eventloopgroup.h/cpp` | Slave 组管理，round-robin 负载均衡 |
| **Connection** | `core/connection.h/cpp` | TCP 连接管理，HTTP 读写 + 生命周期 |
| **Parser** | `http/parser.h/cpp` | HTTP 请求解析器（有限状态机） |
| **Request** | `http/request.h/cpp` | HTTP 请求数据模型 |
| **Router** | `http/router.h/cpp` | 路由分发，支持 `:param` 动态匹配 |
| **ThreadPool** | `thread/threadpool.h/cpp` | 工作者线程池（生产者-消费者） |
| **Socket** | `net/socket.h/cpp` | fd 的 RAII 封装（move-only） |
| **HttpServer** | `server/httpserver.h/cpp` | 服务器主类，组装所有组件 |

---

## 数据流

```
客户端连接
  │
  ▼
Master Reactor: accept → round-robin 选择 Slave
  │
  ▼  [eventfd 跨线程唤醒]
  │
Slave Reactor: 创建 Connection → 加入 epoll
  │
  ▼  [数据到达]
  │
handleRead() → Parser::parse() → Router::dispatch()
  │
  ├── 轻量路由 → conn->mSend()           (当前 I/O 线程)
  └── 耗时路由 → ThreadPool.submit()
                   └── queueInLoop()     (交回 I/O 线程发送)
```

---

## 项目结构

```
src/
├── main.cpp                      # 入口
├── core/
│   ├── channel.h/cpp             # I/O 事件通道
│   ├── eventloop.h/cpp           # 事件循环（基类）
│   ├── slaveeventloop.h/cpp      # 从 Reactor
│   ├── eventloopgroup.h/cpp      # 从 Reactor 组
│   └── connection.h/cpp          # TCP 连接
├── net/
│   └── socket.h/cpp              # Socket RAII 封装
├── http/
│   ├── request.h/cpp             # HTTP 请求模型
│   ├── parser.h/cpp              # HTTP 解析器
│   └── router.h/cpp              # 路由分发
├── server/
│   └── httpserver.h/cpp          # 服务器封装
├── handler/
│   └── routeregister.h/cpp       # 路由注册
├── thread/
│   └── threadpool.h/cpp          # 线程池
└── utils/
    └── stringutil.h/cpp          # 工具函数
```

---

## License

MIT
