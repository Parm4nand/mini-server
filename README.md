# Multithreaded C++ HTTP Web Server

A lightweight, high-performance HTTP/1.1 web server built from scratch in C++17 using raw POSIX sockets and a custom worker thread pool. Serves static assets and dynamic portfolio content without relying on external web libraries or heavy dependencies.

---

## Features

- **POSIX Sockets:** Low-level TCP socket manipulation (`socket`, `bind`, `listen`, `accept`).
- **Thread Pool Architecture:** Implements a Producer-Consumer pattern using `std::thread`, `std::mutex`, and `std::condition_variable` to handle concurrent client requests efficiently.
- **Static File Serving:** Reads and streams `index.html` and static files using binary file streams (`std::ifstream`).
- **HTTP/1.1 Protocol Support:** Parses HTTP GET requests and returns standardized HTTP status codes (`200 OK`, `404 Not Found`, `405 Method Not Allowed`).

---

## System Architecture
```Architecture
                    [ Incoming TCP Connections ]
                                │
                                ▼
                    ┌───────────────────────┐
                    │ Main Listener Thread  │ (Produces socket file descriptors)
                    └───────────┬───────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │   Thread-Safe Queue   │ (Protected by mutex & condition variable)
                    └───────────┬───────────┘
                                │
        ┌───────────────────────┼───────────────────────┐
        ▼                       ▼                       ▼
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│ Worker Thread 1 │     │ Worker Thread 2 │     │ Worker Thread N │ (Consumers: Parses HTTP,
└─────────────────┘     └─────────────────┘     └─────────────────┘  streams response, closes socket)
```
---

##  Project Structure

```text
.
├── main.cpp    # Primary C++ source (POSIX Sockets & ThreadPool implementation)
├── index.html    # Static HTML document served to web clients
└── README.md     # Project documentation
```

## Build & Execution Instructions

## Prerequisites

* **OS**: Linux / macOS (POSIX-compliant environment)

* **Compiler**: g++ or clang++ with C++17 support

* **Tools**: make (optional), curl

## Compilation

 Build the executable using g++:

``` bash
g++ -std=c++17 main.cpp -o server -pthread
```
---
## Running the Server
Ensure index.html is present in the same directory as the executable, then start the server:
```bash
./server
```
The server will initialize on http://localhost:8080.

---

## Testing & Verification
1. Web Browser:
Navigate to http://localhost:8080 to view the served index.html page.

2. Terminal (curl):
```bash
curl -i http://localhost:8080/
``` 
