#include "Network.hpp"

#include <memory>
#include <thread>
#include <iostream>

#include "Node.hpp"

Network::Network() :
    m_nodes(),
    timescale(0.01f) {

}

void runtime(Network *network, std::shared_ptr<Node> node) {
    node->runtime(network);
}

std::shared_ptr<Node> Network::add_node(std::string address, std::shared_ptr<Node> node) {
    node->address = address;
    std::cout << "Setting address to " << address << std::endl;
    m_nodes[address] = node;
    node->thread = std::thread(runtime, this, m_nodes[address]);

    return m_nodes[address];
}

Node* Network::get_node(Node *fallback, std::string address) {
    if(address.empty()) {
        return fallback;
    }

    return m_nodes[address].get();
}

void Network::reply(NetworkMessage* respondingTo, Message *reply) {
    // cannot reply to response
    if(respondingTo->is_response()) {
        return;
    }

    m_mutex.lock();
    if(respondingTo->to() == "0.0.0.0") {

    } else {
        m_sending.push_back(MessageSending(new NetworkMessage(respondingTo->id(), respondingTo->to(), respondingTo->from(), reply, true)));
    }
    m_mutex.unlock();
}


uint32_t Network::send_message(std::string from, std::string to, Message *message) {
    m_mutex.lock();
    uint32_t idx = get_mesg_idx();
    m_sending.push_back(MessageSending(new NetworkMessage(idx, from, to, message)));
    m_mutex.unlock();

    return idx;
}

void Network::update() {
    m_mutex.lock();
    for(int i = 0; i < m_sending.size(); i++) {
        auto& sending = m_sending[i];
        if(sending.update(timescale)) {
            // append message to target
            if(m_nodes.count(sending.target())) {
                m_nodes[sending.target()]->append_message((NetworkMessage*)sending.message());
            }

            m_sending.erase(m_sending.begin() + i);
            i--;
        }
    }
    m_mutex.unlock();
}
