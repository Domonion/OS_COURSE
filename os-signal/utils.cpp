#include "utils.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
using std::cout;
using std::endl;
using std::string;

void sprint(const string & s) {
	check_error(write(1, s.c_str(), s.size()), "sprint");
}

void check_error(int rc, const string &additional) {
    if (rc == -1) {
        int error = errno;
	if(write(1, additional.c_str(), additional.size()) == -1 || 
        write(1, strerror(error), strlen(strerror(error))) == -1) {
		//something went totally wrong
		exit(-1);
		}
        exit(0);
    }
}
