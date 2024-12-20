#include "Mutex.hpp"
#include "network/Network.hpp"
#include <chrono>
#include <thread>

Mutex::Mutex(std::string sheriff) :
m_sheriff(sheriff),
m_is_working(false) {

}


void Mutex::handle_message(NetworkMessage* message) {
    switch(message->message()->message_type()) {
        case 1:
            break;
        case 2:
            start_work();
            break;
        case 3:
            break;
    }
}

void Mutex::solve_messages() {
    NetworkMessage *message = nullptr;
    while((message = receive_message())) {
        handle_message(message);
    }
}

void Mutex::start_work() {
    m_is_working = true;
    m_started_working = std::chrono::system_clock::now();
    m_work_for_ms = std::chrono::milliseconds(rand() % 10);
}

void Mutex::finish_work() {
    m_is_working = false;
    m_work_for_ms = std::chrono::milliseconds(0);
    send_message(m_network, m_sheriff, new MutexReleaseMessage());
}

bool Mutex::do_work() {
    auto now = std::chrono::system_clock::now();
    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_started_working) > m_work_for_ms / *m_network->timescale_ptr()) {
        finish_work();

        return true;
    }

    return false;
}

void Mutex::runtime(Network *network) {
    m_network = network;

    while(true) {
        solve_messages();

        if(m_is_working) {
            do_work();
        }
    }
}

void Mutex::request() {
    send_message(m_network, m_sheriff, new MutexRequestMessage());
}





void MutexSheriff::handle_message(NetworkMessage* message) {
    switch(message->message()->message_type()) {
        case 1:
            if(m_worker.has_value()) {
                push(message->from());
            } else {
                set_worker(message->from());
            }
            break;
        case 2:
            break;
        case 3:
            next_worker();
            break;
    }
}

void MutexSheriff::solve_messages() {
    NetworkMessage *message = nullptr;
    while((message = receive_message())) {
        handle_message(message);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


void MutexSheriff::push(std::string worker) {
    m_queue.push_back(worker);
}

std::string MutexSheriff::pop() {
    auto result = m_queue.front();
    m_queue.pop_front();
    return result;
}

MutexSheriff::MutexSheriff() {

}

void MutexSheriff::runtime(Network *network) {
    m_network = network;

    while(true) {
        solve_messages();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
