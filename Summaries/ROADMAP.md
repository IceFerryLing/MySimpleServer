# 🗺️ 项目全景路线图 (Project Roadmap)

本文档详细规划了从基础 TCP 通信到现代全栈 C++ 开发的学习与实施路径。

---

## 🟢 第一阶段：基础网络通信 (Basic Networking)
**状态**: ✅ 已完成

本阶段旨在掌握 Boost.Asio 的核心概念，建立稳定的 TCP 收发能力。

### 核心成果
- **同步服务器 (Sync Server)**: 理解阻塞 IO 模型，线程每连接 (Thread-per-connection)。
- **异步服务器 (Async Server)**:
    - **v1 (Simple)**: 理解 `async_read`/`async_write` 回调机制，Proactor 模式。
    - **v2 (Robust)**: 
        - **粘包处理**: 实现 `Header + Body` 协议，解决 TCP 流式传输问题。
        - **发送队列**: 使用 `std::queue` 保证 `async_write` 的串行调用，避免并发写入冲突。
        - **字节序**: 处理 Big-Endian / Little-Endian 转换。

### 📚 知识点回顾
> 🔗 **详细总结**: [第一阶段总结：基础网络通信与 Modern C++ (Phase1_Summary.md)](Phase1_Summary.md)

- [x] **TCP 协议**: 三次握手，流式传输，MSS/MTU。
- [x] **Boost.Asio**: `io_context`, `socket`, `endpoint`, `buffer`。
- [x] **内存管理**: `std::shared_ptr` 管理 Session 生命周期 (`shared_from_this`)。

---

## 🟡 第二阶段：序列化与协议 (Serialization & Protocols)
**状态**: 🚧 计划中

本阶段将引入标准化的数据交换格式，替代原始的二进制结构体。

### 🛠️ 技术栈
- **Protobuf**: Google 高效二进制序列化协议。
- **JSON**: `nlohmann/json` 用于文本协议。

### 🗺️ 实施方案
1.  **集成 Protobuf**: 编写 `.proto` 文件，生成 C++ 类，替换现有的 `MsgNode` 数据部分。
2.  **集成 JSON**: 实现一个简单的配置加载器，或用于非频发的控制指令。
3.  **协议分发**: 设计 `Dispatcher`，根据消息 ID 自动调用对应的处理函数。

### 📚 核心知识点
- [ ] **序列化原理**: TLV (Tag-Length-Value) 编码。
- [ ] **反射机制**: C++ 静态反射 (通过模板或宏实现消息映射)。

---

## 🟠 第三阶段：架构设计与优化 (Architecture)
**状态**: 📅 待办

本阶段关注服务器的性能、稳定性和代码解耦。

### 🗺️ 实施方案
1.  **逻辑层解耦**: 
    - 引入 `LogicSystem` 单例。
    - 网络层只负责收发数据，将完整包投递到逻辑队列。
    - 逻辑线程从队列取包处理，实现 IO 与 业务分离。
2.  **多线程模型**:
    - **IO 线程池**: 多个 `io_context` 运行在不同线程，通过 `round-robin` 分发新连接。
    - **逻辑线程池**: 处理耗时业务，防止阻塞 IO 线程。
3.  **优雅退出**: 捕获 `SIGINT`/`SIGTERM` 信号，安全关闭 socket 和 `io_context`。

### 📚 核心知识点
- [ ] **并发编程**: 锁 (`std::mutex`), 条件变量 (`std::condition_variable`), 原子操作 (`std::atomic`).
- [ ] **设计模式**: 单例模式 (Singleton), 生产者-消费者模型.

---

## 🔴 第四阶段：进阶技术 (Advanced)
**状态**: 📅 长期目标

探索 C++ 新特性与微服务架构。

### 🗺️ 实施方案
1.  **C++20 协程 (Coroutines)**:
    - 使用 `co_await` 改造异步回调地狱，让异步代码写起来像同步代码。
2.  **RPC 框架**:
    - 集成 gRPC，实现服务间调用。

---

## 🔵 扩展阶段：前端交互与可视化 (Web Integration)
**状态**: 🚀 重点推进

本阶段的目标是将 C++ 后端与现代 Web 前端技术结合，实现可视化的交互界面。

### 🛠️ 技术栈 (Tech Stack)
- **后端**: C++20, `Boost.Beast` (HTTP/WebSocket), `nlohmann/json`.
- **前端**: HTML5, CSS3 (Flexbox), JavaScript (ES6+, WebSocket API).
- **工具**: Chrome DevTools, Postman.

### 🗺️ 详细实施方案 (Implementation Strategy)

#### Step 1: 引入 Boost.Beast 与 HTTP 服务
- **目标**: 浏览器访问 `http://localhost:8080` 能显示 "Hello Beast"。
- **建议**: 参考 `http_server_async.cpp`，实现静态文件服务 (读取 `index.html` 返回)。

#### Step 2: WebSocket 握手与回显 (Echo)
- **目标**: 实现 C++ 后端与 JS 前端的第一次握手。
- **建议**: 检测 HTTP `Upgrade: websocket` 头，升级连接。使用 `beast::websocket::stream`。

#### Step 3: JSON 协议设计与解析
- **目标**: 前后端通过 JSON 交互。
- **建议**: 定义 `{ "type": "msg", "content": "..." }` 格式，后端封装 `JsonHandler`。

#### Step 4: 前端界面开发
- **目标**: 可视化聊天室。
- **建议**: 左侧在线列表，右侧聊天框。使用原生 JS + CSS Flex 布局。

### 📚 核心知识点清单 (Knowledge Checklist)

#### 1. 网络协议
- [ ] **HTTP/1.1**: Methods (GET/POST), Status Codes (200/404), Headers.
- [ ] **WebSocket**: Handshake (Upgrade), Frames (Text/Binary), Ping/Pong.

#### 2. C++ 后端 (Beast)
- [ ] **核心对象**: `http::request`, `http::response`, `websocket::stream`.
- [ ] **异步安全**: `shared_from_this()` 保活, `strand` 保证线程安全.

#### 3. 前端基础
- [ ] **HTML/CSS**: Flexbox 布局, 基础标签.
- [ ] **JavaScript**: `new WebSocket()`, `onmessage`, `JSON.parse/stringify`.

### 📺 推荐学习资料
- **C++ 网络**: [Boost.Asio 网络编程 (恋恋风辰)](https://space.bilibili.com/271469206)
- **计算机网络**: [计算机网络微课堂 (湖科大)](https://www.bilibili.com/video/BV1c4411d7jb)
- **前端基础**: [尚硅谷 2023版 HTML+CSS](https://www.bilibili.com/video/BV1844y1r7iF) / [JS](https://www.bilibili.com/video/BV1WM4y127s9)
