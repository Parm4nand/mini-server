#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

constexpr int PORT = 8080;
constexpr size_t THREAD_POOL_SIZE = 4;

class ThreadPool {
public:
    explicit ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    int client_fd;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->cv.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
                        
                        client_fd = this->tasks.front();
                        this->tasks.pop();
                    }
                    handle_client(client_fd);
                }
            });
        }
    }

    void enqueue(int client_fd) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push(client_fd);
        }
        cv.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        cv.notify_all();
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<int> tasks;
    std::mutex queue_mutex;
    std::condition_variable cv;
    bool stop;

    void handle_client(int client_fd) {
        char buffer[1024] = {0};
        read(client_fd, buffer, sizeof(buffer) - 1);

        std::istringstream request_stream(buffer);
        std::string method, path, version;
        request_stream >> method >> path >> version;

        std::string body;
        std::string response;

        if (method == "GET") {
            std::string file_path = (path == "/" || path == "/index.html") ? "index.html" : path.substr(1);
            std::ifstream file(file_path, std::ios::binary);

            if (file.is_open()) {
                std::stringstream file_stream;
                file_stream << file.rdbuf();
                body = file_stream.str();

                response = "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + std::to_string(body.length()) + "\r\n"
                    "Connection: close\r\n\r\n" + body;
            } else {
                body = "<html><body><h1>404 Not Found</h1></body></html>";
                response = "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + std::to_string(body.length()) + "\r\n"
                    "Connection: close\r\n\r\n" + body;
            }
        }

        send(client_fd, response.c_str(), response.length(), 0);
        close(client_fd);
    }
};

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        return 1;
    }

    std::cout << "Server running on http://localhost:" << PORT << std::endl;

    ThreadPool pool(THREAD_POOL_SIZE);

    while (true) {
        sockaddr_in client_addr{};
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        pool.enqueue(client_fd);
    }

    close(server_fd);
    return 0;
}
