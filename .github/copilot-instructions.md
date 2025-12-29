# Copilot Instructions for MySimpleServer

This project is a C++ network programming workspace focused on learning and implementing synchronous and asynchronous servers using `Boost.Asio`.

## Project Architecture

- **Sync/**: Contains the synchronous implementation.
  - `SyncServer.cpp`: Implements a thread-per-connection server model. Uses `std::thread` detached for each session.
  - `SyncClient.cpp`: A blocking client implementation.
- **pre_learn/**: Contains educational code snippets and foundational concepts.
  - `endpoint/endpoint.cpp`: Demonstrates `Boost.Asio` buffer management (`const_buffer`, `mutable_buffer`) and endpoint handling.
- **Aync/**: Intended for asynchronous implementation (currently under development).

## Build & Run

- **Compiler**: MinGW-w64 `g++`.
- **Standard**: C++20 (`-std=c++20`).
- **Dependencies**:
  - `Boost.Asio` (Header-only usage observed).
  - `easyxw` (Linked in build tasks, though usage is currently minimal/absent).
  - Windows System Libraries: `gdi32`, `user32`, `kernel32`, `ws2_32` (implied by `winsock2.h`).
- **Tasks**: Use the VS Code task "C/C++: g++.exe 生成活动文件" to build the active file.

## Coding Conventions

### Network Programming
- **Boost.Asio**: The primary networking library.
- **Windows Sockets**: The client code (`SyncClient.cpp`) explicitly manages `WSAStartup` and `WSACleanup`. Maintain this pattern if modifying the client or creating similar standalone Windows clients.
- **Buffers**: Prefer `std::vector<char>` or `std::string` managed buffers over raw arrays when possible, though raw arrays (`char data[MAX_LENGTH]`) are used in current simple examples.

### Error Handling
- Use `try-catch` blocks for top-level exception handling.
- Use `boost::system::error_code` for specific Asio operation error checking (e.g., `sock->read_some(..., error)`).

### Style & Documentation
- **Language**: Comments and documentation should be in **Chinese** (Simplified).
- **Headers**: Use standard headers (`<iostream>`, `<memory>`, `<thread>`) and Boost headers (`<boost/asio.hpp>`).
- **Namespaces**: `using namespace std;` and `using namespace boost::asio::ip;` are commonly used in implementation files.

## Key Workflows

1.  **Building**: Open the target `.cpp` file (e.g., `SyncServer.cpp`) and run the default build task.
2.  **Testing**:
    - Run the Server executable first.
    - Run the Client executable in a separate terminal.
    - Client connects to `127.0.0.1:10086`.

## Known Issues / Notes
- `AyncApi.cpp` is currently empty.
- The build task hardcodes include paths (`C:\mingw64_15\mingw64\include`). Ensure your environment matches or update `tasks.json` if paths differ.
