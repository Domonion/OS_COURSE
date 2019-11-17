#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <zconf.h>
#include <utils.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <sys/un.h>
#include <cstring>

using std::cout;
using std::endl;
using std::mt19937;
using std::vector;
using std::string;

int main(int argc, char **argv) {
    mt19937 randomer(std::chrono::system_clock::now().time_since_epoch().count());
    int s = socket(AF_UNIX, SOCK_STREAM, 0);
    check_error(s, "socket");
    struct sockaddr_un server{};
    server.sun_family = AF_UNIX;
    strncpy(server.sun_path, PATH.c_str(), PATH.size());
    check_error(connect(s, (sockaddr *) (&server), sizeof(server)), "connect");
    int p_in, p_out;
    p_in = recv_fd(s);
    p_out = recv_fd(s);
    char len;
    doRead(&len, 1, p_in);
    vector<char> res;
    res.resize(len);
    doRead(&res[0], len, p_in);
    for(char i : res)
	    cout << i;
    cout << endl;
    res.resize(randomer() % MAX_MESSAGE_LEN + 1);
    len = res.size();
    for(char & i : res) {
	    i = randomer() % 26 + 'a';
	    cout << i;
    }
    cout << endl;
    doWrite(&len, 1, p_out);
    doWrite(&res[0], len, p_out);
    check_error(shutdown(s, SHUT_RDWR), "shutdown");
    check_error(close(s), "close");
    check_error(close(p_in), "p_in");
    check_error(close(p_out), "p_out");
    return 0;
}
