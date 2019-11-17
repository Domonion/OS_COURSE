#include "static.hpp"
#include "dynamic_linked.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <iostream>

using std::cerr;

int main() {
    _static::msg();
    dynamic_linked::msg();
    void *handle = dlopen("./lib_dynamic_not_linked.so", RTLD_LAZY);
    if (handle == nullptr) {
        cerr << dlerror();
        return 0;
    }
    auto msg = (void (*)()) (dlsym(handle, "msg"));
    if (msg == nullptr) {
        cerr << dlerror();
        return 0;
    }
    msg();
    if (dlclose(handle))
        cerr << dlerror();
    return 0;
}