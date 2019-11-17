#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <errno.h>
#include <zconf.h>
#include <cstring>
#include <string>
#include <vector>
#include "utils.hpp"
#include <random>
#include <sys/un.h>
#include <chrono>
#include <sys/stat.h>
#include <fcntl.h>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::string;
using std::mt19937;

int main(int argc, char **argv) {
    mt19937 randomer(std::chrono::system_clock::now().time_since_epoch().count());
    int master = socket(AF_UNIX, SOCK_STREAM, 0);
    check_error(master, "socket");
    struct stat stat_buf;
    if (stat(PATH.c_str(), &stat_buf) != -1) {
        check_error(unlink(PATH.c_str()), "unlink");
    }
    struct sockaddr_un server{};
    memset(&server, 0, sizeof(sockaddr_un));
    server.sun_family = AF_UNIX;
    strncpy(server.sun_path, PATH.c_str(), PATH.size());
    socklen_t size = sizeof(sockaddr_un);
    check_error(bind(master, (sockaddr *) (&server), size), "bind");
    check_error(listen(master, SOMAXCONN), "listen");
    while (true) {
        int slave = accept(master, NULL, NULL);
        check_error(slave, "accept");
        int p_in[2], p_out[2];
        check_error(pipe(p_in), "p_in");
        check_error(pipe(p_out), "p_out");
        send_fd(slave, p_in[0]);
        send_fd(slave, p_out[1]);
        vector<char> msg(randomer() % MAX_MESSAGE_LEN + 1);
        for (char &i : msg) {
            i = randomer() % 26 + 'a';
            cout << i;
        }
        cout << endl;
        char len = msg.size();
        doWrite(&len, 1, p_in[1]);
        doWrite(&msg[0], len, p_in[1]);
        doRead(&len, 1, p_out[0]);
        msg.resize(len);
        doRead(&msg[0], len, p_out[0]);
        for (char i : msg) {
            cout << i;
        }
        cout << endl;
        check_error(shutdown(slave, SHUT_RDWR), "shutdown");
        check_error(close(slave), "close");
        check_error(close(p_in[0]), "p_in");
        check_error(close(p_in[1]), "p_in");
        check_error(close(p_out[0]), "p_out");
        check_error(close(p_out[1]), "p_out");
    }
}
