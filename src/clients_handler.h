#ifndef SOCKETCHAT_CLIENTS_HANDLER_H
#define SOCKETCHAT_CLIENTS_HANDLER_H

#include <string>
void client_handler(int client_fd);
void add_client(int fd, const std::string &name);
void update_client(int fd, const std::string &new_name);
void remove_client(int fd);
void broadcast(int sender_fd, const std::string &msg);
std::string string_cleaning(char *buffer, int bytes_recv);

#endif //SOCKETCHAT_CLIENTS_HANDLER_H