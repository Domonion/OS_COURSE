#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <cstring>
#include <utils.hpp>
#include <string>
#include <unistd.h>
#include <vector>

using std::string;
using std::vector;
using std::to_string;

const vector<string> registors{ "CR2", 
"CSGSFS", 
"EFL", 
"ERR", 
"OLDMASK", 
"R10", 
"R11", 
"R12", 
"R13", 
"R14", 
"R15", 
"R8", 
"R9", 
"RAX", 
"RBP", 
"RBX", 
"RCX", 
"RDI", 
"RDX", 
"RIP", 
"RSI", 
"RSP", 
"TRAPNO"
};

const vector<int> indexes{REG_CR2,
REG_CSGSFS,
REG_EFL,
REG_ERR,
REG_OLDMASK,
REG_R10,
REG_R11,
REG_R12,
REG_R13,
REG_R14,
REG_R15,
REG_R8,
REG_R9,
REG_RAX,
REG_RBP,
REG_RBX,
REG_RCX,
REG_RDI,
REG_RDX,
REG_RIP,
REG_RSI,
REG_RSP,
REG_TRAPNO 
};

string itos(uint64_t reg) {
	string res;
	for(int i = 0; i < 16; i++) {
		res.push_back('0' + reg % 16);
		reg /= 16;
		if(i % 2) { 
			res.push_back(' ');
		}
	}
	for(char & i : res) {
		if(i != ' ') {
			i = (i > '9' ? 'A' + i - '9' - 1 :  i);
		}
	}
	return res;
}

string reformat(string s) {
	string required(33 - s.size(), ' ');
	s.insert(s.begin(), required.begin(), required.end());
	return s;
}

bool test(char * ptr, int pp[2]) {
	return write(pp[1], ptr, 1) != -1;
}

void handle(int sig, siginfo_t *info, void *ucontext) {
	struct ucontext_t * context = static_cast<ucontext_t *>(ucontext);
	for(int i = 0; i < indexes.size(); i++) {
		sprint(reformat(registors[i] + " " + itos(context->uc_mcontext.gregs[indexes[i]]) + "\n"));
	}
	string error = "--";
	int pp[2];
	check_error(pipe(pp), "pipe");
	for(int i = -30; i <= 30; i++) {
		char *now = static_cast<char*>(info->si_addr) + i;
		if(test(now, pp)) {
			string output;
			char buf;
			check_error(read(pp[0], &buf, 1), "read"); 
			uint8_t res = buf;
			output.push_back('0' + res / 16);
			output.back() = output.back() > '9' ? 'A' + output.back() - '9' - 1 : output.back();
			output.push_back('0' + res % 16);
			output.back() = output.back() > '9' ? 'A' + output.back() - '9' - 1 : output.back();
			sprint(output);
		} else{
			sprint(error);
		}
	}
	sprint("\n");
	check_error(close(pp[0]), "close1");
	check_error(close(pp[1]), "close2");
	exit(-1);	
}

int main() {
	struct sigaction act;
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = handle;
	check_error(sigemptyset(&act.sa_mask), "sigempty");
	check_error(sigaction(SIGSEGV, &act, NULL), "sigaction");
	int * test = reinterpret_cast<int*>(main);
        //test -= 100500;;
	//test -= 11725;
	//test += 9780;
	//test += 100500;
	//test = nullptr;
       	*test = 228;	
    	return 0;
}	
