#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>
#include <atomic>
#include <poll.h>
#include "clients_handler.h"

using namespace std;

atomic<bool> should_run{true};

void sigint_handler(int signum) {
    should_run = false;
}

int main() {
    signal(SIGINT, sigint_handler);

    int server_fd{socket(AF_INET, SOCK_STREAM, 0)};
    if (server_fd == -1) {
        perror("Server socket creation failed");
        exit(1);
    }
    int opt{1};
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);

    if (::bind(server_fd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
        perror("Server bind failed");
        exit(1);
    }

    if (::listen(server_fd, 10) < 0) {
        perror("Server listen failed");
        exit(1);
    }

    pollfd pollfd{};
    pollfd.fd = server_fd;
    pollfd.events = POLLIN;

    cout << "\t Server started on port 8080\n";
    cout << "\t Waiting for client to connect...\n";

    while (should_run) {
        int ret{poll(&pollfd, 1, 1000)};
        if (ret < 0) {
            if (errno == EINTR) { continue; }
            perror("Server poll failed");
            break;
        }

        if (ret == 0) { continue; }

        if (pollfd.revents & POLLIN) {
            int client_fd{accept(server_fd, nullptr, nullptr)};
            if (client_fd >= 0) {
                cout << "[SERVER_INFO]: New connection:  (fd: " << client_fd << ")\n";
                thread(client_handler, client_fd).detach();
            } else {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) { continue; }
                perror("Accept failed");
            }
        }
    }
    cout << "\n\t[SERVER_INFO]: Shutting down clean...\n";
    close(server_fd);
    return 0;
}
