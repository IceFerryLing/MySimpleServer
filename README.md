# Boost.Asio 网络编程学习项目

这是一个基于 C++20 和 Boost.Asio 的网络编程学习仓库。项目旨在从零开始构建高性能的 TCP 服务器，涵盖同步阻塞模式到异步非阻塞模式的演进过程。

> 📺 **配套视频教程**: [C++ 网络编程 (Bilibili)](https://space.bilibili.com/271469206/channel/collectiondetail?sid=1623216&spm_id_from=333.788.0.0)

## 🏗️ 项目状态 (Project Status)
**开发中 (Work In Progress)**
目前已完成同步服务器和基础异步服务器的实现，正在进行架构优化和功能扩展。

## ⏱️ 学习进度 (Current Progress)
- **当前章节**: [C++ 网络编程(9) 字节序处理和发送队列控制](https://www.bilibili.com/video/BV1tF411h7r6)
- **已掌握知识点**:
    - 发送队列 (`std::queue`) 保证异步写操作的串行化。
    - 消息节点 (`MsgNode`) 管理数据生命周期。
    - 解决 TCP 粘包问题 (Header + Body 协议).

## 🗂️ 目录结构说明 (Directory Structure)

```text
asio-network-study/
├── .github/
│   └── copilot-instructions.md # Copilot 配置文件
├── .vscode/
│   └── tasks.json              # VS Code 编译任务配置
├── Async/                      # 异步非阻塞模型 (Proactor Pattern)
│   ├── v1_Simple/              # [版本1] 简单的半双工实现
│   │   ├── AsyncServer.cpp     # 程序入口
│   │   ├── README.md           # 版本说明
│   │   ├── Server_demo.cpp     # 服务器类实现
│   │   ├── Server_demo.h       # 服务器类声明
│   │   ├── Session_demo.cpp    # 会话类实现 (读写逻辑)
│   │   └── Session_demo.h      # 会话类声明
│   ├── v2_FullDuplex/          # [版本2] 健壮的全双工实现 (推荐)
│   │   ├── AsyncServer.cpp     # 程序入口
│   │   ├── MsgNode.cpp         # 消息节点实现
│   │   ├── MsgNode.h           # 消息节点声明 (RAII)
│   │   ├── README.md           # 版本说明
│   │   ├── Server_demo.cpp     # 服务器类实现
│   │   ├── Server_demo.h       # 服务器类声明
│   │   ├── Session_demo.cpp    # 会话类实现 (读写分离)
│   │   └── Session_demo.h      # 会话类声明
│   ├── AsyncClient/            # 异步客户端实现
│   │   ├── main.cpp            # 客户端入口 (含发送线程)
│   │   ├── AsyncClient.cpp     # 客户端核心类实现
│   │   └── AsyncClient.h       # 客户端核心类声明
│   └── README.md               # Async 模块总说明
├── pre_learn/                  # 基础概念验证与代码片段
│   ├── endpoint/               # 端点与缓冲区
│   │   ├── endpoint.cpp
│   │   ├── endpoint.h
│   │   └── README.md
│   └── Session/                # Session 类原型
│       ├── README.md
│       ├── Session.cpp
│       └── Session.h
├── Sync/                       # 同步阻塞模型 (Thread-Per-Connection)
│   ├── README.md               # Sync 模块说明
│   ├── SyncClient.cpp          # 同步客户端
│   ├── SyncClient学习版.cpp     # 带注释的客户端代码
│   ├── SyncServer.cpp          # 同步服务器
│   └── SyncServer学习版.cpp     # 带注释的服务器代码
└── README.md                   # 项目总说明
```

### 1. [Sync/](Sync/) - 同步阻塞模型
- 最基础的 C/S 模型。
- **特点**: 一线程一连接 (Thread-Per-Connection)。
- **适用**: 低并发、简单的测试场景。

### 2. [Async/](Async/) - 异步非阻塞模型 (核心)
- 基于 Boost.Asio 的 Proactor 模式实现。
- **[v1_Simple](Async/v1_Simple/)**: 简单的半双工实现（存在缺陷，仅供反面教材）。
- **[v2_FullDuplex](Async/v2_FullDuplex/)**: 全双工、带发送队列的健壮实现（推荐参考）。
- **[AsyncClient](Async/AsyncClient/)**: 异步客户端，支持多线程发送和接收。

### 3. [pre_learn/](pre_learn/) - 基础概念验证
- 包含 Endpoint、Buffer 等基础知识的小型测试代码。

## 💻 开发环境 (Environment)
- **系统**: Windows
- **编译器**: MinGW-w64 g++ (C++20)
- **依赖库**: Boost (Header-only + System/Thread/Context)
- **工具**: VS Code

## ▶️ 如何运行 (How to Run)
本项目配置了 VS Code 的 `tasks.json`。
1. 打开任意 `.cpp` 文件（如 `Async/v2_FullDuplex/AsyncServer.cpp`）。
2. 按下 `Ctrl + Shift + B` 运行构建任务。
3. 在终端运行生成的 `.exe` 文件。

## 🧠 学习重点 (Key Takeaways)

### 1. 内存与生命周期管理
- **智能指针**: 在异步编程中，回调函数执行时对象必须存活。通过 `std::shared_ptr` 和 `std::enable_shared_from_this` (伪闭包技术)，确保 `Session` 对象在所有异步操作完成前不被销毁。
- **RAII**: 使用 `MsgNode` 管理发送/接收缓冲区的生命周期，避免内存泄漏和野指针。

### 2. 并发模型对比
- **同步 (Sync)**: 简单直观，但每个连接占用一个线程，资源消耗大，不适合高并发。
- **异步 (Async)**: 基于事件驱动 (Proactor)，单线程即可处理成千上万个连接。难点在于逻辑被回调函数割裂，需要维护上下文。

### 3. 网络协议设计
- **粘包与半包**: TCP 是流式协议。必须设计应用层协议（如 `Header + Body`）来界定消息边界。
- **全双工通信**: 读写分离。接收循环 (`HandleRead`) 和发送循环 (`HandleWrite`) 独立运行，互不阻塞。

### 4. Boost.Asio 最佳实践
- **IO Context**: 核心调度器。通常一个程序一个 `io_context`，或者使用 `io_context` 线程池。
- **Strand**: 虽然本项目暂未使用，但在多线程操作同一个 socket 时，`strand` 是保证线程安全的重要工具（本项目通过队列+单IO线程保证安全）。

## ✅ 待办事项 (To-Do List)

### 🟢 第一阶段：基础网络通信 (已完成)
- [x] **同步服务器**: 基础 Echo Server 实现。
- [x] **异步服务器 (v1)**: 基础收发功能。
- [x] **异步服务器 (v2)**: 
    - [x] 解决 TCP 粘包 (Header + Body)。
    - [x] 发送队列 (保证 `async_write` 串行化)。
    - [x] 字节序处理 (Big-Endian/Little-Endian)。
- [x] **异步客户端**: 多线程收发架构。

### 🟡 第二阶段：序列化与协议 (计划中)
- [ ] **序列化**: 集成 Protobuf (Google Protocol Buffers)。
- [ ] **序列化**: 集成 JSON (用于配置文件或简单通信)。
- [ ] **应用层协议**: 简单的 HTTP 服务器实现 (Asio)。

### 🟠 第三阶段：架构设计与优化
- [ ] **逻辑层架构**: 封装 `LogicSystem` (单例模式)，实现业务逻辑与网络层解耦。
- [ ] **优雅退出**: 实现服务器的安全关闭机制 (信号处理)。
- [ ] **多线程模型**: 
    - [ ] `IOServicePool`: 多 `io_context` 线程池模式。
    - [ ] `IOThreadPool`: 单 `io_context` 多线程模式。

### 🔴 第四阶段：进阶技术 (长期目标)
- [ ] **C++20 协程**: 使用 Coroutines 简化异步代码 (Co_await)。
- [ ] **Beast 网络库**: 实现高性能 HTTP/WebSocket 服务器。
- [ ] **RPC 框架**: 集成 gRPC 进行微服务通信。
