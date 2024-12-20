#pragma once

#include "NetworkIO.hpp"
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <msd/channel.hpp>
#include <iostream>

class Network;
class Node;
class Message;

/**
 * How message looks in the network.
 * Contains info about sender, receiver and unique id;
 */
class NetworkMessage {
    uint32_t m_id;

    std::string m_from;
    std::string m_to;

    Message *m_message;
    bool m_isResponse;

public:
    const std::string& from() { return m_from; }
    const std::string& to() { return m_to; }
    const uint32_t id() { return m_id; }
    const Message* message() { return m_message; }
    const bool is_response() { return m_isResponse; }

    NetworkMessage(uint32_t id, std::string from, std::string to, Message* message, bool is_response = false) :
        m_id(id),
        m_from(from),
        m_to(to),
        m_message(message),
        m_isResponse(is_response) {

    }

    const std::string display() const { return m_message->display(); }
};

class FutureReply {
    std::string m_target;
    uint32_t m_id;
    NetworkMessage *m_response;

public:
    inline const std::string& target() const { return m_target; }
    inline const uint32_t id() { return m_id; }

    FutureReply(uint32_t id, std::string target) :
        m_id(id),
        m_target(target),
        m_response(nullptr) {

    }

    void reply(NetworkMessage *response) {
        m_response = response;
    }

    bool is_replied() {
        return m_response != nullptr;
    }

    NetworkMessage *response() {
        return m_response;
    }
};



static std::chrono::milliseconds time_to_send_message = std::chrono::milliseconds(10);

class MessageSending {
    float m_state;
    NetworkMessage *m_message;

    std::chrono::steady_clock::time_point last;


public:
    const std::string& target() { return m_message->to(); }
    const std::string& from() { return m_message->from(); }
    float state() { return m_state; }
    const NetworkMessage* message() { return m_message; }

    const bool is_response() {return m_message->is_response();}

    MessageSending(NetworkMessage *message) :
        m_state(0),
        m_message(message),
        last(std::chrono::steady_clock::now()) {
    }

    float update(float timescale) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        // adds time between begin and end to elapsed
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - last);
        last = end;
        m_state += elapsed.count() / time_to_send_message.count() * timescale;

        return m_state >= 1.0f;
    }
};


class Network {
private:
    std::map<std::string, std::shared_ptr<Node>> m_nodes;

    std::vector<MessageSending> m_sending;
    std::mutex m_mutex;

    float timescale;

    uint32_t m_mesgIdx = 1;


public:
    float* timescale_ptr() { return &timescale; }
    uint32_t get_mesg_idx() { return m_mesgIdx++; }

    std::vector<MessageSending>& messages() { return m_sending; }
    const std::map<std::string, std::shared_ptr<Node>>& map() { return m_nodes; }

    Network();

    Node* get_node(Node *node, std::string address);

    std::shared_ptr<Node> add_node(std::string address, std::shared_ptr<Node> node);

    void reply(NetworkMessage* respondingTo, Message *reply);

    uint32_t send_message(std::string from, std::string to, Message *message);

    void update();
};
