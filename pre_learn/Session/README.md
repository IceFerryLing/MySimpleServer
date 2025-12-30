# Session 类学习笔记

这个目录下的 `Session.h` 和 `Session.cpp` 是用于学习 Boost.Asio 异步读写操作的核心示例代码。它展示了如何正确封装一个 TCP 会话，处理数据的发送队列、生命周期管理以及 TCP 拆包/粘包的基础处理（通过缓冲区管理）。

## 核心类说明

### 1. MsgNode (消息节点)

`MsgNode` 是一个简单的消息容器，用于管理待发送数据的生命周期。

*   **设计目的**：在异步发送中，底层缓冲区（Buffer）必须在异步操作完成之前保持有效。如果使用局部变量或临时变量，回调执行时数据可能已被销毁。
*   **实现细节**：
    *   使用 `char* _msg` 存储数据（深拷贝）。
    *   `_total_len`：消息总长度。
    *   `_cur_len`：当前已发送/处理的长度。这对于处理 `async_write_some` 的部分发送情况至关重要。
    *   通常配合 `std::shared_ptr` 使用，确保引用计数不为 0，直到回调函数执行完毕。

### 2. Session (会话类)

`Session` 类封装了 `asio::ip::tcp::socket`，并提供了多种读写模式的演示。

#### 关键机制：发送队列 (`_send_queue`)

这是 Asio 网络编程中非常重要的一个模式。

*   **问题**：Boost.Asio 的 `socket` 不是完全线程安全的。特别是，**不能在同一个 socket 上并发发起多个异步写操作**（`async_write` 或 `async_write_some`）。如果前一个写操作未完成就发起下一个，行为是未定义的（通常会导致崩溃或数据错乱）。
*   **解决方案**：
    *   使用 `std::queue<std::shared_ptr<MsgNode>> _send_queue` 缓存待发送消息。
    *   `WriteToSocket` 函数只负责将消息入队。
    *   如果当前没有正在进行的发送任务 (`!_send_pending`)，则发起第一次异步写。
    *   在回调函数 (`WriteCallBack`) 中，检查队列是否还有剩余消息。如果有，继续发起下一次异步写。
    *   这样保证了同一时刻只有一个写操作在进行。

#### 方法对比

代码中演示了几种不同的实现方式，用于对比学习：

1.  **错误/演示版 (`WriteToSocketErr`)**：
    *   直接调用 `async_write_some`。
    *   **缺点**：没有使用队列。如果连续快速调用多次，会违反 Asio 的并发写限制。
    *   **用途**：仅用于演示如何处理 `bytes_transferred < total_len` 的部分发送情况。

2.  **标准版 (`WriteToSocket` + `WriteCallBack`)**：
    *   使用 `_send_queue` 保证顺序。
    *   使用 `async_write_some`。
    *   **细节**：回调中需要手动判断 `_cur_len` 是否等于 `_total_len`。如果未发完，需要递归调用 `async_write_some` 发送剩余部分（偏移指针）。

3.  **全量版 (`WriteAllToSocket` + `WriteAllCallBack`)**：
    *   使用 `async_send` (对应 Asio 的 `async_write` 高级封装)。
    *   **特点**：`async_send` 内部会自动循环调用底层写操作，直到所有数据发送完毕或出错才调用回调。
    *   **优点**：简化了用户代码，不需要手动处理部分发送的循环。

4.  **读取操作 (`ReadFromSocket` / `ReadAllFromSocket`)**：
    *   演示了如何发起异步读。
    *   注意：当前的实现逻辑是试图填满整个 `RECVSIZE` 缓冲区。在实际业务中，通常需要解析消息头长度，而不是死板地读取固定长度。

## 学习重点

1.  **生命周期**：观察 `std::shared_ptr` 是如何在 `std::bind` 中被捕获，从而延长对象生命周期的。
2.  **异步链**：理解 "发起操作 -> 回调 -> (判断是否完成) -> 发起下一个操作" 的链式处理流程。
3.  **错误处理**：每个回调都检查了 `boost::system::error_code`。

## 待改进之处 (思考题)

*   当前的 `ReadCallBack` 逻辑是必须读满 `RECVSIZE` (1024字节) 才算结束，这在处理变长消息时是不合理的。如何改进为 "先读头部长度，再读包体"？
*   `MsgNode` 使用了 `new char[]`，可以考虑改用 `std::vector<char>` 或内存池来优化内存分配。
