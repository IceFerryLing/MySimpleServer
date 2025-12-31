# Boost.Asio 网络编程学习项目

这是一个基于 C++20 和 Boost.Asio 的网络编程学习仓库。项目旨在从零开始构建高性能的 TCP 服务器，涵盖同步阻塞模式到异步非阻塞模式的演进过程。

> 📺 **配套视频教程**: [C++ 网络编程 (Bilibili)](https://space.bilibili.com/271469206/channel/collectiondetail?sid=1623216&spm_id_from=333.788.0.0)

## 🚧 项目状态
**开发中 (Work In Progress)**
目前已完成同步服务器和基础异步服务器的实现，正在进行架构优化和功能扩展。

## 📂 目录结构说明

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

### 3. [pre_learn/](pre_learn/) - 基础概念验证
- 包含 Endpoint、Buffer 等基础知识的小型测试代码。

## 🛠️ 开发环境
- **系统**: Windows
- **编译器**: MinGW-w64 g++ (C++20)
- **依赖库**: Boost (Header-only + System/Thread/Context)
- **工具**: VS Code

## 🚀 如何运行
本项目配置了 VS Code 的 `tasks.json`。
1. 打开任意 `.cpp` 文件（如 `Async/v2_FullDuplex/AsyncServer.cpp`）。
2. 按下 `Ctrl + Shift + B` 运行构建任务。
3. 在终端运行生成的 `.exe` 文件。

## 🌟 学习重点 (Key Takeaways)

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
- **串行化写**: 同一个 Socket 不能并发调用 `async_write`。必须使用 **发送队列** (`std::queue`) + **互斥锁** (`std::mutex`) 来串行化发送请求。
- **IO Context**: 理解 `io_context` 作为任务调度器的核心作用，以及 `run()` 循环的机制。

## 💡 学习总结 (Summary)

从同步到异步的跨越，本质上是思维模式的转变：从 **"等待操作完成"** 转变为 **"发起操作并注册回调"**。

1.  **v1 版本**教会了我们异步的基本写法，但也暴露了半双工逻辑在复杂场景下的脆弱性（如死锁、响应延迟）。
2.  **v2 版本**通过引入**发送队列**和**消息协议**，解决了实际工程中最棘手的两个问题：**并发写冲突**和**TCP粘包**。这是迈向生产级代码的关键一步。
3.  **AsyncClient** 的实现则展示了如何在客户端也应用同样的异步思想，实现高效的全双工交互。

掌握这些模式后，你不仅能熟练使用 Boost.Asio，更能深入理解高性能网络服务器的底层架构原理（如 Nginx, Node.js 的事件循环）。

## 📝 待办事项 (Todo)
- [x] Socket 的监听和连接
- [x] Buffer 结构和同步读写
- [x] 同步读写 Server 和 Client 示例
- [x] 异步读写 API 介绍和使用
- [x] 官方案例异步 Server 及隐患
- [x] 利用伪闭包延长连接的生命周期
- [x] 增加发送队列实现全双工通信
- [ ] 处理粘包问题
- [ ] 字节序处理和发送队列控制
- [ ] 采用 Protobuf 序列化
- [ ] 采用 Json 序列化
- [ ] Asio 粘包处理的简单方式
- [ ] 逻辑层架构概述
- [ ] 利用单例逻辑实现逻辑类
- [ ] 服务器优雅退出
- [ ] Asio 多线程模型 IOServicePool
- [ ] Asio 另一种多线程模型 IOThread
- [ ] 使用 Asio 协程搭建异步服务器
- [ ] 利用协程实现并发服务器 (上/下)
- [ ] Asio 实现 HTTP 服务器
- [ ] Beast 网络库实现 HTTP 服务器
- [ ] Beast 网络库实现 WebSocket 服务器
- [ ] Windows 环境下 gRPC 配置和编译
- [ ] 利用 gRPC 通信
- [ ] 数据库连接池

---
*最后更新: 2025-12-31*
