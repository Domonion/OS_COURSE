#include "utils.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/socket.h>
#include <unistd.h>

using std::cout;
using std::endl;
using std::string;

void check_error(int rc, const string &additional) {
    if (rc == -1) {
        int error = errno;
        cout << additional << endl;
        cout << strerror(error) << endl;
        exit(0);
    }
}

void doWrite(char *what, int amount, int where) {
    int counter = 0;
    while (counter < amount) {
        int sent = write(where, what + counter, amount - counter);
        check_error(sent, "send");
        counter += sent;
    }
}

void doRead(char *where, int amount, int from) {
    int counter = 0;
    while (counter < amount) {
        int received = read(from, where + counter, amount - counter);
        check_error(received, "recv");
        counter += received;
    }
}

void send_fd(int socket_fd, int fd) {
    char buf[CMSG_SPACE(sizeof(fd))];
    memset(buf, 0, sizeof(buf));
    struct iovec io = {.iov_base = (void *) "KEK", .iov_len = 3};
    msghdr msg{};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
    *((int *) CMSG_DATA(cmsg)) = fd;
    msg.msg_controllen = CMSG_LEN(sizeof(fd));
    check_error(sendmsg(socket_fd, &msg, 0), "send_fd");
}

int recv_fd(int socket_fd) {
    char buffer[256];
    char control_buf[256];
    struct iovec io = {.iov_base = buffer, .iov_len = sizeof(buffer)};
    msghdr msg{};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);
    check_error(recvmsg(socket_fd, &msg, 0), "recvmsg");
    cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    return *((int *) CMSG_DATA(cmsg));
}