//
// Created by domonion on 18.04.19.
//
#include <iostream>
#include "_dynamic_not_linked.hpp"

extern "C" {
    void msg() {
        std::cout << "This is dynamic, but not linked lib" << std::endl;
    }
}