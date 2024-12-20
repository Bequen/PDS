#pragma once

#include "Log.hpp"
#include <chrono>
#include <optional>
#include <string>
#include <msd/channel.hpp>
#include <thread>

class Network;
class NetworkMessage;
class Message;
class Response;
class FutureReply;

class MessageInfo {
    std::string m_from;

    uint32_t m_type;
    std::string m_display;

    bool m_isResponse;

    std::chrono::steady_clock::time_point m_time;

public:
    const std::string& from() const { return m_from; }
    const uint32_t type() const { return m_type; }
    const std::string& display() const { return m_display; }
    const bool is_response() const { return m_isResponse; }
    const std::chrono::steady_clock::time_point time() { return m_time; }

    MessageInfo(std::string from, uint32_t type, std::string display, bool isResponse) :
        m_from(from),
        m_type(type),
        m_display(display),
        m_isResponse(isResponse) {
        m_time = std::chrono::steady_clock::now();
    }
};

enum NodeState {
    RUNNING,
    STOPPED,
};

class Node {
protected:
    NodeState m_state;
    NodeLog m_log;
    std::vector<MessageInfo> m_receivedMessages;

    msd::channel<NetworkMessage*> channel;
    std::vector<std::shared_ptr<FutureReply>> m_future_replies;

    NetworkMessage* receive_message();

    std::shared_ptr<FutureReply> send_message(Network *network, std::string target, Message* message);

    std::optional<std::shared_ptr<FutureReply>> pop_response();

public:
    inline NodeState state() const { return m_state; }
    inline void set_state(NodeState state) { m_state = state; }

    NodeLog& log() { return m_log; }
    const std::vector<MessageInfo>& received_messages() const {
        return m_receivedMessages;
    }

    Node() :
        channel(10),
        m_state(RUNNING) {

    }
    std::thread thread;
    std::string address;

    Network *m_network;

    void run(Network *network);

    void stop() {
        m_state = STOPPED;
    }

    void run() {
        m_state = RUNNING;
    }

    void runtime_wrapper(Network *network) {
        if(m_state != STOPPED) {
            runtime(m_network);
        }
    }

    virtual void runtime(Network* network) = 0;

    void append_message(NetworkMessage *message);
};
