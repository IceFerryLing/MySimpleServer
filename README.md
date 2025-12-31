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
