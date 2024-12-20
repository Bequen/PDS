#pragma once

#include "network/NetworkIO.hpp"
#include "network/Node.hpp"
#include <chrono>
#include <string>
#include <iostream>


class MutexRequestMessage : public Message {
public:
    unsigned int message_type() const {
        return 1;
    }

    std::string display() {
        return "Request";
    }
};

class MutexOkMessage : public Message {
public:
    unsigned int message_type() const {
        return 2;
    }

    std::string display() {
        return "Ok";
    }
};

class MutexReleaseMessage : public Message {
public:
    unsigned int message_type() const {
        return 3;
    }

    std::string display() {
        return "Release";
    }
};


class Mutex : public Node {
private:
    std::string m_sheriff;
    bool m_is_working;
    uint32_t m_remaining_work;
    std::chrono::system_clock::time_point m_started_working;
    std::chrono::milliseconds m_work_for_ms;

    void handle_message(NetworkMessage *message);

    void solve_messages();

    void start_work();
    void finish_work();

    bool do_work();

public:
    inline bool is_working() const { return m_is_working; }
    uint32_t remaining_work() const {return m_remaining_work;}

    Mutex(std::string sheriff);

    void runtime(Network* network) override;

    void request();
};

class MutexSheriff : public Node {
    std::deque<std::string> m_queue;
    std::optional<std::string> m_worker;

    void push(std::string worker);
    std::string pop();

    void handle_message(NetworkMessage *message);

    void solve_messages();

    void set_worker(std::string worker) {
        m_worker = worker;
        send_message(m_network, worker, new MutexOkMessage());
    }

    void next_worker() {
        if(has_next_worker()) {
            set_worker(pop());
        } else {
            m_worker = {};
        }
    }

public:
    inline bool has_next_worker() const {return !m_queue.empty(); }

    inline const std::deque<std::string> queue() { return m_queue; }
    inline const std::optional<std::string>& worker() const { return m_worker; }

    MutexSheriff();

    void runtime(Network* network) override;
};
