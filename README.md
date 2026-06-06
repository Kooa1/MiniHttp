# MiniHttp

一个基于 **Linux epoll** 的 C++17 HTTP 服务器，采用 **主从 Reactor（Master-Slave Multi-Reactor）** 架构，内置线程池，支持中间件管道和静态文件服务。

## 架构

```
┌────────────────────────────────────────────────────────────┐
│               Master Reactor (main thread)                 │
│  EventLoop: accept 连接 + 信号处理                          │
└─────────────────────────────┬──────────────────────────────┘
                              │ round-robin 分发 client_fd
          ┌───────────────────┼────────────────────┐
          ▼                   ▼                    ▼
┌──────────────────┐ ┌──────────────────┐ ┌───────────────────┐
│ SlaveEventLoop 1 │ │ SlaveEventLoop 2 │ │ ... (可配置 N 个)  │
│ epoll fd         │ │ epoll fd         │ │                   │ 
│ Connection set   │ │ Connection set   │ │                   │
│ read/parse/      │ │ read/parse/      │ │                   │ 
│ dispatch/send    │ │ dispatch/send    │ │                   │
└─────────┬────────┘ └─────────┬────────┘ └──────────┬────────┘
          │                    │                     │
          └────────────────────┼─────────────────────┘
                               │ 耗时任务提交
                               ▼
                       ┌────────────────┐
                       │   ThreadPool   │
                       │  (CPU 密集计算) │
                       └────────────────┘
```

- **主从 Reactor**: 1 个 Master Reactor（main 线程，仅负责 accept）+ N 个 Slave Reactor（IO 线程，负责读写/解析/发送），充分利用多核 CPU
- **线程池**: M 个 Worker 线程处理 CPU 密集型计算，IO 线程保持响应
- **跨线程唤醒**: eventfd 实现 IO 线程与 Worker 线程之间的无锁通信

## 特性

- **事件驱动** — 基于 epoll 的边缘触发 IO 多路复用，非阻塞 socket
- **HTTP 解析** — 确定性有限状态机（DFA）解析 HTTP 请求，支持 GET/POST/HEAD
- **动态路由** — 支持参数化路由（`/user/:id/post/:pid`），自动路径参数提取
- **中间件管道** — 插件式请求处理链（日志、鉴权等），洋葱模型
- **静态文件服务** — 基于 `sendfile` 的零拷贝文件传输，支持目录索引 + MIME 类型检测 + 路径遍历防护
- **线程池** — 生产者-消费者模型，Worker 线程处理耗时业务逻辑
- **连接保活** — HTTP/1.1 keep-alive 支持，连接复用
- **连接超时** — timerfd 定时器，周期性检测并关闭空闲连接
- **生命周期管理** — `shared_ptr<bool>` 令牌机制，安全处理跨线程连接关闭竞态
- **优雅关闭** — signalfd 信号处理，逐层反序停止（Slave → ThreadPool）
- **Buffer 设计** — 非连续缓冲区，`readv` + 栈上 extrabuf 减少系统调用
- **RAII 封装** — Socket 等资源使用 move-only RAII 封装，自动管理生命周期

## 构建

```bash
# 依赖: CMake 3.16+, C++17 编译器, Linux (epoll)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 使用

```bash
./minihttp_server

# 默认监听 http://localhost:8080
```

### 命令行参数

在 main.cpp 中配置：

```cpp
HttpServer server(port, io_thread_count, worker_thread_count);
server.setTimeout(seconds);
```

默认值：端口 8080，IO 线程 2，Worker 线程 4，超时 10 秒。

### 注册路由

```cpp
server.Get("/", [](const Request &req, Connection *conn) {
    conn->mSend("Hello World!");
});

server.Get("/user/:id", [](const Request &req, Connection *conn) {
    conn->mSend("User: " + req.param("id"));
});
```

### 使用中间件

```cpp
server.use([](const Request &req, Connection *conn, Middleware::Next next) {
    // 前置处理
    next();  // 调用下一个中间件或路由处理
    // 后置处理
});
```

### 静态文件服务

```cpp
server.ServerStatic("/static", "/path/to/www");
```

## 项目的结构

```
MiniHttp/
├── CMakeLists.txt
└── src/
    ├── main.cpp                       # 入口
    ├── core/                          # 核心事件驱动模块
    │   ├── channel.h/cpp              # IO 事件通道
    │   ├── eventloop.h/cpp            # 事件循环基类（epoll + eventfd）
    │   ├── slaveeventloop.h/cpp       # 从 Reactor（+ 连接管理 + timerfd）
    │   ├── eventloopgroup.h/cpp       # 从 Reactor 组管理（round-robin）
    │   └── connection.h/cpp           # TCP 连接管理
    ├── net/                           # 网络层
    │   ├── socket.h/cpp               # Socket RAII 封装
    │   └── buffer.h/cpp               # 缓冲区（readv + 自动扩容）
    ├── http/                          # HTTP 协议层
    │   ├── request.h/cpp              # HTTP 请求模型
    │   ├── parser.h/cpp               # HTTP 解析器（有限状态机）
    │   ├── router.h/cpp               # 路由分发（+ 参数化路由）
    │   ├── middleware.h/cpp           # 中间件管道
    │   └── staticfilehandler.h/cpp    # 静态文件服务（sendfile）
    ├── server/
    │   └── httpserver.h/cpp           # 服务器封装
    ├── handler/
    │   └── routeregister.h/cpp        # 路由注册
    ├── thread/
    │   └── threadpool.h/cpp           # 线程池
    └── utils/
        └── stringutil.h/cpp           # 路径分割工具
```

## 基准测试

> **测试环境**
>
> | 项目 | 规格 |
> |------|------|
> | 服务器 | Linux 云服务器（2 vCPU / 2 GiB） |
> | 测试工具 | wrk |
> | 测试时长 | 10s |
> | 服务端 IO 线程 | 2 |
> | 服务端 Worker 线程 | 4 |
> | 连接超时 | 10s |
>
> 完整测试数据（28 行）见 [bench-report-20260606-020213.md](bench-report-20260606-020213.md)

| 场景 | 并发 | RPS | 平均延迟 | 最大延迟 | p50 | p99 |
|------|------|-----|---------|---------|-----|-----|
| GET / | 1 | 20,039 | 63.17us | 5.34ms | 39.00us | 438.00us |
| GET / | 10 | 42,355 | 148.23ms | 1.66s | 142.00us | 1.51s |
| GET / | 50 | 43,643 | 189.89ms | 1.95s | 650.00us | 1.79s |
| GET / | 100 | 40,864 | 195.30ms | 1.88s | 1.26ms | 1.75s |
| GET /about | 1 | 17,047 | 29.63ms | 831.08ms | 44.00us | 713.51ms |
| GET /about | 10 | 28,217 | 110.87ms | 1.66s | 115.00us | 1.53s |
| GET /about | 50 | 28,234 | 25.94ms | 756.50ms | 515.00us | 641.33ms |
| GET /about | 100 | 28,508 | 1.75ms | 62.58ms | 0.98ms | 16.52ms |
| GET /slow (线程池) | 1 | 7,783 | 64.09ms | 1.23s | 95.00us | 1.11s |
| GET /slow (线程池) | 10 | 28,562 | 59.59ms | 911.15ms | 248.00us | 811.05ms |
| GET /slow (线程池) | 50 | 28,717 | 128.79ms | 1.28s | 1.14ms | 1.14s |
| GET /slow (线程池) | 100 | 32,279 | 185.80ms | 1.70s | 1.96ms | 1.56s |
| GET static/index.html | 1 | 9,229 | 50.44ms | 1.08s | 81.00us | 967.21ms |
| GET static/index.html | 10 | 25,538 | 164.66ms | 1.57s | 262.00us | 1.43s |
| GET static/index.html | 50 | 24,730 | 170.40ms | 1.50s | 1.28ms | 1.43s |
| GET static/index.html | 100 | 25,585 | 116.48ms | 1.22s | 2.88ms | 1.10s |
| GET /api/routes | 1 | 16,329 | 89.02ms | 1.40s | 44.00us | 1.28s |
| GET /api/routes | 10 | 20,357 | 94.50ms | 1.52s | 126.00us | 1.40s |
| GET /api/routes | 50 | 24,982 | 2.74ms | 177.52ms | 496.00us | 64.75ms |
| GET /api/routes | 100 | 16,637 | 1.81ms | 38.66ms | 848.00us | 18.86ms |
| GET /user/1234 (动态路由) | 1 | 13,944 | 489.21us | 74.04ms | 56.00us | 14.11ms |
| GET /user/1234 (动态路由) | 10 | 21,105 | 211.50ms | 1.94s | 178.00us | 1.80s |
| GET /user/1234 (动态路由) | 50 | 26,494 | 1.39ms | 81.09ms | 677.00us | 16.46ms |
| GET /user/1234 (动态路由) | 100 | 18,539 | 1.79ms | 32.68ms | 1.43ms | 13.27ms |

### 测试结果要点

- **最佳吞吐**: `GET /` 场景在并发 50 时达到 **43,643 RPS**，p99 延迟 1.79s
- **最稳定**: `GET /about` 在并发 100 时 p99 仅 16.52ms，高并发下延迟分布均匀
- **sendfile 效率**: 静态文件服务（`/static/`）RPS 约 25,000，传输吞吐达 76MB/s
- **动态路由**: 参数化路由（`/user/1234`）RPS 约 26,000，p99 低至 13-16ms
- **线程池**: `/slow` 涉及线程池任务提交 + 跨线程回调，RPS 约 32,000
- **无错误**: 所有测试场景 **Non-2xx = 0**，无超时/连接错误（在并发≥50 时有 socket timeout，属于 wrk 预期行为）

## 依赖

- Linux（需要 epoll、eventfd、signalfd、timerfd、sendfile）
- CMake >= 3.16
- C++17 编译器
- pthread（连接超时定时器需要）

## 许可证

MIT