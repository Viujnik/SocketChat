#include <iostream>
#include <thread>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

void receive_message(int socket_fd) {
    char buffer[2048];
    while (true) {
        ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            cout << "\n\t[SERVER_INFO]: Connection lost.\n";
            exit(0);
        }
        buffer[bytes_received] = '\0';
        cout << "\n" << buffer << flush;
    }
}

int main() {
    int socket_fd{socket(AF_INET, SOCK_STREAM, 0)};
    sockaddr_in serv_address{};
    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_address.sin_addr);

    if (connect(socket_fd, reinterpret_cast<sockaddr *> (&serv_address), sizeof(serv_address)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    cout << "Enter your nickname: ";
    string nickname;
    getline(cin, nickname);
    send(socket_fd, nickname.c_str(), nickname.length(), 0);

    thread(receive_message, socket_fd).detach();

    string message;
    while (true) {
        getline(cin, message);
        if (message == "exit") { break; }
        if (send(socket_fd, message.c_str(), message.length(), 0) < 0) {
            perror("Message sending failed");
            break;
        }
    }
    close(socket_fd);
    return 0;
}
