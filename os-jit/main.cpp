#include <iostream>
#include <sys/mman.h>
#include <string>
#include <cstring>

using std::cout;
using std::string;
using std::endl;

void print_error() {
    int error = errno;
    cout << strerror(error) << endl;
}

int main(int argc, char **argv) {
    if (argc > 2) {
        cout << "Usage: " + string(argv[0]) + " [NUMBER]" << endl;
        return 0;
    }
    size_t const code_length = 11;
    size_t const value_index = 5;
    uint8_t code[] = {
            0x55,
            0x48, 0x89, 0xe5,
            0xb8, 0x00, 0x00, 0x00, 0x00,
            0x5d,
            0xc3
    };
    if (argc == 2)
        *reinterpret_cast<int *>(code + value_index) = atoi(argv[1]);
    void *f = mmap(nullptr, 4096, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (f == (void *) -1) {
        print_error();
        return 0;
    }
    memcpy(f, code, code_length);
    if (mprotect(f, code_length, PROT_EXEC) == -1) {
        print_error();
        return 0;
    }
    cout << ((int (*)()) f)() << endl;
    int rc = munmap(f, code_length);
    if (rc == -1)
        print_error();
}