//
// Created by domonion on 02.05.19.
//

#include <string>

#ifndef FIND_UTILS_HPP
#define FIND_UTILS_HPP

void check_error(int rc, const std::string& additional);
void doWrite(char *what, int amount, int where);
void doRead(char *where, int amount, int from);
void send_fd(int socket, int fd);
int recv_fd(int socket);
int const MAX_MESSAGE_LEN = 100;
const std::string PATH = "/tmp/9Lq7BNBnBycd6nxy.socket";
#endif //FIND_UTILS_HPP
