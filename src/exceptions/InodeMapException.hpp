#pragma once
#include <iostream>

class InodeMapException: public std::exception {
    std::string message;

public:
    InodeMapException(const std::string& message): message(message) { }
    const char* what() const noexcept override { return message.c_str();}
};
