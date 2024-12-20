#pragma once

#include <chrono>
#include <csignal>
#include <string>
#include <vector>

struct LogMessage {
    std::chrono::system_clock::time_point time;
    std::string message;
};

class NodeLog {
private:
    std::vector<LogMessage> messages;

public:
    size_t size() {
        return messages.size();
    }

    LogMessage& get(uint32_t idx) {
        return messages[idx];
    }

    void log(std::string message) {
        LogMessage msg;
        msg.time = std::chrono::system_clock::now();
        msg.message = message;
        messages.push_back(msg);
    }
};
