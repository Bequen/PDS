#include "Node.hpp"

#include "Network.hpp"

NetworkMessage* Node::receive_message() {
    NetworkMessage* out = nullptr;
    if(!channel.empty()) {
        channel >> out;
    }
    return out;
}

std::optional<std::shared_ptr<FutureReply>> Node::pop_response() {
    for(int i = 0; i < m_future_replies.size(); i++) {
        if(m_future_replies[i]->is_replied()) {
            auto reply = m_future_replies[i];
            m_future_replies.erase(m_future_replies.begin() + i);
            return reply;
        }
    }

    return {};
}

std::shared_ptr<FutureReply> Node::send_message(Network *network, std::string target, Message* message) {
    uint32_t id = network->send_message(this->address, target, message);
    auto future_reply = std::make_shared<FutureReply>(id, target);
    m_future_replies.push_back(future_reply);
    return future_reply;
}


void Node::run(Network* network) {
    this->m_network = network;
    runtime(this->m_network);
}

void Node::append_message(NetworkMessage *message) {
    m_receivedMessages.push_back(MessageInfo(message->from(), message->message()->message_type(), message->display(), message->is_response()));
    if(message->is_response()) {
        for(auto& reply : m_future_replies) {
            if(reply->id() == message->id()) {
                reply->reply(message);
            }
        }
    } else {
        channel << message;
    }
}
