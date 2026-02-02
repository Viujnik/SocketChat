#include <iostream>
#include <map>
#include <mutex>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

static map<int, string> clients;
static mutex clients_mutex;

static bool isNameTaken(const string &name) {
    for (auto const &[fd, client_name]: clients) {
        if (name == client_name) return true;
    }
    return false;
}

void add_client(const int fd, const string &name) {
    lock_guard lock(clients_mutex);
    string unique_name = name;
    if (isNameTaken(name)) {
        unique_name = name + "_" + to_string(fd);
        string warning = "Server: Name " + name + " taken, you are assigned as " + unique_name + "\n";
        send(fd, warning.c_str(), warning.size(), 0);
    }
    clients[fd] = unique_name;
}

void update_client(const int fd, const string &new_name) {
    lock_guard lock(clients_mutex);
    if (isNameTaken(new_name)) {
        string error_msg = "Server: Name " + new_name + " is already taken\n";
        send(fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    if (clients.count(fd)) {
        clients[fd] = new_name;
        string info_msg = "Server: Name has been changed to " + new_name + "\n";
        send(fd, info_msg.c_str(), info_msg.size(), 0);
    }
}

void remove_client(const int fd) {
    lock_guard lock(clients_mutex);
    auto iter = clients.find(fd);
    if (iter != clients.end()) {
        cout << "[Log]: Client " << iter->second << " (fd: " << fd << ") disconnected.\n";
        clients.erase(iter);
    }
    close(fd);
}

void broadcast(const int sender_fd, const string &msg) {
    lock_guard lock(clients_mutex);
    string sender_name = "Unknown";
    if (clients.count(sender_fd)) {
        sender_name = clients[sender_fd];
    }
    string sender_msg = "[" + sender_name + "]: " + msg + "\n";
    for (auto const &[fd, name]: clients) {
        if (sender_fd != fd) {
            send(fd, sender_msg.c_str(), sender_msg.size(), 0);
        }
    }
}

string string_cleaning(char *buffer, const int bytes_recv) {
    buffer[bytes_recv] = '\0';
    string some_string = buffer;
    if (!some_string.empty() && some_string.back() == '\n' || some_string.back() == '\r') {
        some_string.pop_back();
    }
    return some_string;
}

void client_handler(int client_fd) {
    char buffer[1024];
    int bytes_recv = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_recv <= 0) {
        remove_client(client_fd);
        return;
    }
    string username = string_cleaning(buffer, bytes_recv);
    add_client(client_fd, username);
    broadcast(client_fd, " has joined the chat!");

    while (true) {
        bytes_recv = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_recv <= 0) { break; }
        string message = string_cleaning(buffer, bytes_recv);

        if (message.substr(0, 6) == "/name ") {
            update_client(client_fd, message.substr(6));
        } else {
            broadcast(client_fd, message);
        }
    }
}
