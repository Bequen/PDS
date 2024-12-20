#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <sstream>
#include <unistd.h>
#include <vector>

#include "../network/NetworkIO.hpp"
#include "../network/Node.hpp"

class ChordMessage : public Message {
private:
    int m_target_hash_value;

public:
    int target_hash_value() { return m_target_hash_value; }

    void set_target_hash_value(uint32_t hash_value) { m_target_hash_value = hash_value; }

    ChordMessage(int target_hash_value)
        : m_target_hash_value(target_hash_value),
         Message() {

    }
};

class ChordAddNodeMessage final : public ChordMessage {
public:
    std::string address;
    int hash_value;

    ChordAddNodeMessage(std::string address, int hash_value) :
        address(address),
        hash_value(hash_value),
        ChordMessage(hash_value) {

    }

    virtual unsigned int message_type() const override {
        return 1;
    }

    virtual std::string display() override {
        auto stringBuilder = std::stringstream();
        stringBuilder << "Add node at [" << address << "]" << " with hash [" << hash_value << "]";
        return stringBuilder.str();
    }
};


class ChordGetFingersMessage : public ChordMessage {
public:
    std::string start_address;
    std::vector<std::string> fingers;

    int start;
    int i;

    ChordGetFingersMessage(int start) :
        start(start),
        ChordMessage(0) {
        i = 1;
    }

    ChordGetFingersMessage(int start, std::string start_address) :
        start(start),
        start_address(start_address),
        i(1),
        ChordMessage(0) {
    }

    virtual unsigned int message_type() const override {
        return 3;
    }

    virtual std::string display() override {
        return "Get fingers";
    }
};

class ChordJumpMessageResponse : public ChordMessage {
public:
    std::string m_content;

    ChordJumpMessageResponse(std::string content) :
        m_content(content),
        ChordMessage(0) {

    }

    virtual unsigned int message_type() const override {
        return 4;
    }

    virtual std::string display() override {
        return "Jump response";
    }
};

class ChordJumpMessage : public ChordMessage {
    std::string m_content;

public:
    std::string& content() { return m_content; }

    ChordJumpMessage(uint32_t hash_value, std::string content) :
        m_content(content),
        ChordMessage(hash_value) {

    }

    virtual unsigned int message_type() const override {
        return 4;
    }

    virtual std::string display() override {
        return "Chord finger";
    }
};

class Chord : public Node {
private:
    int m_k;
    int m_hash_value;

    std::string m_next;
    int m_next_hash_value;

    std::vector<std::string> m_fingers;
    bool is_fingers_built;
    bool m_sentForBuild;

    uint32_t m_messageIdx;

    std::vector<std::shared_ptr<FutureReply>> m_replies;

public:
    bool *automatically_build_fingers = new bool(true);

    int hash_value() { return m_hash_value; }

    int min() { return 0; }
    int max() { return std::pow(2,m_k) - 1;}

    Chord(int k, int hash_value) :
        m_k(k),
        m_hash_value(hash_value),
        m_next_hash_value(hash_value),
        is_fingers_built(false),
        m_messageIdx(0),
        Node() {

    }

    std::vector<std::string>& get_fingers() {
        return m_fingers;
    }

    void set_fingers(std::vector<std::string> fingers) {
        m_fingers = fingers;
        is_fingers_built = true;
    }

    void set_next(std::string address, int hash_value) {
        m_next = address;
        m_next_hash_value = hash_value;

        std::cout << "Setting next of [" << this->address << "]" << " to " << "[" << address << "]" << std::endl;
    }

    std::string get_next_address() {
        return m_next.empty() ? address : m_next;
    }

    int distance(int a, int b) {
        if (a == b) {
            return 0;
        } else if (a < b) {
            return b - a;
        } else {
            return max() + 1 - a + b;
        }
    }

    void print_finger_table() {
        std::cout << "Finger table" << std::endl;
        int i = 1;
        for (auto &finger : get_fingers()) {
            std::cout << i << " + " << this->hash_value() << " => " << finger << std::endl;
            i *= 2;
        }
    }

    void build_finger_table(Network* network);

    void runtime(Network *network) override;

    // attempts to add `node` to `this` node
    void add_node(Network *network, std::string address, int hash_value);

    std::string get_finger_for_hash(int hash_value) {
        int idx = log2(hash_value);
        return get_fingers()[idx];
    }

    void send_chord_message(Network *network, ChordMessage* message);
};
