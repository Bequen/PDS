#pragma once

#include <string>

class Message {
public:
    virtual unsigned int message_type() const = 0;
    virtual std::string display() = 0;
};
