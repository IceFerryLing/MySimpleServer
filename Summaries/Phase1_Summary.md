# 🟢 第一阶段总结：基础网络通信与 Modern C++

本文档对第一阶段（同步/异步服务器开发）所涉及的核心知识点、设计模式以及 Modern C++ 语法进行了详细梳理。

## 1. 网络编程基础 (理论篇)

### 1.1 TCP/IP 协议栈
*   **应用层 (Application)**: HTTP, FTP, WebSocket, 自定义协议 (本项目重点)。
*   **传输层 (Transport)**: TCP (可靠, 面向连接), UDP (不可靠, 无连接)。本项目基于 **TCP**。
*   **网络层 (Network)**: IP (寻址)。

### 1.2 Socket 核心概念
*   **Socket**: 网络通信的句柄，本质是内核的一个文件描述符 (File Descriptor)。
*   **IP 地址**: 标识网络中的主机 (如 `127.0.0.1`)。
*   **端口 (Port)**: 标识主机上的进程 (如 `10086`)。
*   **五元组**: 唯一标识一个连接 `{源IP, 源端口, 目的IP, 目的端口, 协议}`。

### 1.3 TCP 关键特性
*   **面向连接**: 通信前需建立连接 (三次握手)，通信后需断开 (四次挥手)。
*   **可靠传输**: 确认应答 (ACK)、超时重传、流量控制。
*   **流式传输 (Byte Stream)**: 数据像水流一样没有边界，导致了 **粘包/半包** 问题 (后续章节解决)。

## 2. Boost.Asio 核心机制

### 2.1 I/O 模型对比
*   **同步阻塞 (Synchronous Blocking)**:
    *   **特点**: `read`/`write` 函数会一直阻塞直到操作完成。
    *   **模型**: Thread-Per-Connection (一连接一线程)。
    *   **缺点**: 线程资源消耗大，上下文切换开销高，无法应对高并发。
    *   **代码**: `SyncServer.cpp` 使用 `std::thread` 为每个连接创建独立线程。
*   **异步非阻塞 (Asynchronous Non-Blocking)**:
    *   **特点**: `async_read`/`async_write` 立即返回，操作完成后通过回调函数通知。
    *   **模型**: Proactor 模式 (基于 `io_context` 事件循环)。
    *   **优点**: 单线程即可处理成千上万并发连接，资源利用率极高。
    *   **代码**: `AsyncServer.cpp` 使用 `io_context.run()` 驱动所有异步事件。

### 2.2 核心组件
*   **`boost::asio::io_context`**: 程序的“心脏”，负责调度异步事件和回调函数。
*   **`boost::asio::ip::tcp::socket`**: 封装了底层的 TCP socket，提供 `async_read_some`, `async_write` 等接口。
*   **`boost::asio::ip::tcp::endpoint`**: 表示网络端点 (IP + Port)。
*   **`boost::asio::buffer`**: 适配器，将数组、vector 等转换为 Asio 可用的缓冲区对象 (`const_buffer` / `mutable_buffer`)。

### 2.3 生命周期管理 (关键)
在异步编程中，最容易出现的问题是 **Socket 析构后回调函数才执行** (导致野指针崩溃)。
*   **解决方案**: `std::enable_shared_from_this` + `std::shared_ptr`。
*   **原理**: 在发起异步操作（如 `async_read`）时，将 `Session` 对象的 `shared_ptr` 绑定到回调函数中。这增加了引用计数，确保只要回调函数未执行，`Session` 对象就不会被销毁。

## 3. 底层原理深入 (Deep Dive)

### 3.1 用户态与内核态 (User Space vs Kernel Space)
*   **概念**: 操作系统为了安全，将内存分为用户空间（应用程序运行）和内核空间（操作系统核心运行）。
*   **I/O 过程**: 网络数据到达网卡 -> 拷贝到内核缓冲区 -> 拷贝到用户缓冲区。
*   **开销**: 传统的 `read`/`write` 涉及多次上下文切换和内存拷贝，高性能网络编程致力于减少这些开销（如 Zero-Copy 技术）。

### 3.2 五种 I/O 模型
1.  **阻塞 I/O (Blocking I/O)**: 也就是 `SyncServer` 使用的模型。进程在等待数据时被挂起，无法处理其他任务。
2.  **非阻塞 I/O (Non-blocking I/O)**: 进程发起 `read`，如果没数据立即返回错误 (EWOULDBLOCK)，进程需要不断轮询 (Polling)，浪费 CPU。
3.  **I/O 多路复用 (I/O Multiplexing)**: 使用 `select`/`poll`/`epoll` 监听多个 socket。当某个 socket 就绪时，通知进程去处理。这是 Nginx、Redis 等高性能服务器的基石。
4.  **信号驱动 I/O (Signal Driven I/O)**: 较少使用。
5.  **异步 I/O (Asynchronous I/O)**: 进程发起 `read` 后立即返回，内核负责将数据拷贝到用户缓冲区，完成后通知进程。这是真正的“异步”。

### 3.3 Reactor vs Proactor 模式
Boost.Asio 封装了这两种模式，对外提供统一的异步接口。
*   **Reactor (反应堆)**:
    *   **机制**: "来了事件我通知你，你自己去读写"。
    *   **对应**: Linux `epoll` (默认行为)。
    *   **流程**: `epoll_wait` 返回可读事件 -> 用户线程调用 `read` 将数据从内核拷到用户态。
*   **Proactor (前摄器)**:
    *   **机制**: "我帮你读写好了，数据放在你指定的 buffer 里，然后通知你"。
    *   **对应**: Windows `IOCP` (Input/Output Completion Port)。
    *   **流程**: 用户发起 `async_read` -> 内核自动拷贝数据 -> 完成后调用回调函数。
*   **Asio 的做法**: 在 Windows 上使用原生 IOCP (Proactor)；在 Linux 上使用 epoll 模拟 Proactor (即 Reactor + 用户态模拟)。

## 4. 网络编程实战技巧

### 4.1 TCP 粘包与半包处理
TCP 是流式协议，没有“消息边界”的概念。
*   **现象**: 发送 "Hello" 和 "World"，接收端可能一次收到 "HelloWorld" (粘包) 或 "Hel" (半包)。
*   **解决方案**: **Header + Body 协议**。
    *   固定长度的消息头 (Header)，包含消息体长度信息。
    *   先读头部，解析出长度 `N`，再读取 `N` 字节的 Body。
*   **实现**: `MsgNode` 类用于管理变长消息内存。

### 4.2 发送队列 (Write Queue)
Boost.Asio 的 `async_write` 不能并发调用（即在前一个 `async_write` 未完成前，不能对同一个 socket 发起新的 `async_write`）。
*   **问题**: 如果业务逻辑层频繁发送消息，直接调用 `async_write` 会导致数据错乱或崩溃。
*   **解决方案**:
    1.  引入 `std::queue<MsgNode>`。
    2.  发送时，先将消息入队。
    3.  如果队列之前为空，则发起 `async_write`。
    4.  在 `HandleWrite` 回调中，检查队列是否还有数据，若有则继续发送。
*   **并发安全**: 使用 `std::mutex` 保护队列的入队/出队操作。

### 4.3 字节序 (Endianness)
*   **网络字节序**: Big-Endian (大端)。
*   **主机字节序**: 通常是 Little-Endian (小端，如 x86 架构)。
*   **处理**: 在发送长度信息 (int/short) 前，需使用 `htons`/`htonl` 转换；接收后需使用 `ntohs`/`ntohl` 转换。

## 5. Modern C++ (C++11/14/17/20) 语法应用

本项目大量使用了 C++11 之后的特性来简化代码和提高安全性。

### 5.1 智能指针 (Smart Pointers)
彻底告别 `new`/`delete`，防止内存泄漏。
*   **`std::shared_ptr`**: 共享所有权。用于 `Session` 对象（被 Server 和 Asio 回调共同持有）和 `MsgNode`（被队列和逻辑层共同持有）。
*   **`std::make_shared`**: 创建 `shared_ptr` 的推荐方式，比 `new` 更高效（一次内存分配）。
*   **`std::unique_ptr`**: 独占所有权。虽然本项目主要用 shared_ptr，但在不需要共享资源的场景下应优先使用。

### 5.2 RAII 与 锁
*   **RAII (资源获取即初始化)**: `MsgNode` 在构造时分配内存，析构时释放内存。
*   **`std::lock_guard`**: 自动管理互斥锁 `std::mutex`。
    ```cpp
    {
        std::lock_guard<std::mutex> lock(_send_lock); // 自动上锁
        _send_queue.push(msg);
    } // 作用域结束，自动解锁
    ```
    避免了手动 `unlock` 可能因异常或逻辑错误导致的死锁。

### 5.3 函数对象与回调
*   **`std::bind`**: 将成员函数与对象指针绑定，生成可调用的函数对象，作为 Asio 的回调。
    ```cpp
    std::bind(&Session::HandleRead, this, placeholders::_1, ...)
    ```
*   **Lambda 表达式** (推荐): 比 `std::bind` 更直观、性能更好（编译器易优化）。
    ```cpp
    // 现代写法示例
    socket_.async_read_some(buffer, 
        [self = shared_from_this()](error_code ec, size_t length) {
            self->HandleRead(ec, length);
        });
    ```

### 5.4 其他特性
*   **`auto`**: 自动类型推导，简化迭代器或复杂模板类型的书写。
*   **`std::thread`**: C++11 标准线程库，跨平台替代 `pthread` 或 Windows API。
*   **`std::move`**: 移动语义。在将 `MsgNode` 放入队列时，如果支持移动构造，可以避免内存拷贝（本项目目前主要传递指针，未来优化可涉及）。
