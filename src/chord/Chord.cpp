#include "Chord.hpp"

#include "../network/Network.hpp"

void Chord::build_finger_table(Network* network) {
    m_sentForBuild = true;
    network->send_message(address, get_next_address(), new ChordGetFingersMessage(hash_value(), address));
}

void Chord::add_node(Network *network, std::string address, int hash_value) {
    if(distance(this->hash_value(), hash_value) <= distance(this->m_next_hash_value, hash_value)) {
        if(m_next != address || hash_value != m_hash_value) {
            auto message = new ChordAddNodeMessage(get_next_address(), m_next_hash_value);
            this->set_next(address, hash_value);

            // Trying to set the new node's next value
            network->send_message(this->address, address, message);
        }
    } else if(distance(this->m_next_hash_value, hash_value) != 0) {
        network->send_message(this->address, this->get_next_address(), new ChordAddNodeMessage(address, hash_value));
    }
}


void Chord::send_chord_message(Network *network, ChordMessage* message) {
    network->send_message(this->address, get_finger_for_hash(message->target_hash_value()), message);
}


void Chord::runtime(Network *network) {
    m_network = network;
    int i = 0;
    while(true) {
        auto message = receive_message();
        if(message) {
            auto chordMessage = (ChordMessage*)message->message();
            int message_type = chordMessage->message_type();
            switch(message_type) {
                case 1: {
                    auto msg = (ChordAddNodeMessage*)chordMessage;
                    add_node(network, msg->address, msg->hash_value);
                    break;
                } case 3: {
                    auto msg = (ChordGetFingersMessage*)chordMessage;
                    if(msg->start_address == address) {
                        for(int i = msg->fingers.size(); i <= m_k; i++) {
                            msg->fingers.push_back(this->address);
                        }

                        set_fingers(msg->fingers);
                        m_sentForBuild = false;
                    } else {
                        while(hash_value() >= msg->start + msg->i) {
                            msg->i *= 2;
                            msg->fingers.push_back(this->address);
                        }
                        network->send_message(this->address, this->get_next_address(), msg);
                    }
                    break;
                } case 4: {
                    auto msg = (ChordJumpMessage*)chordMessage;
                    int idx = floor(log2(msg->target_hash_value() - hash_value()));

                    if(msg->target_hash_value() < m_next_hash_value || idx == 0) {
                        m_log.log(msg->content());
                        network->reply(message, new ChordJumpMessageResponse("test"));
                        std::cout << "Found" << std::endl;
                        break;
                    }

                    i = 1;
                    std::cout << "Jumping to: " << idx << " because " << msg->target_hash_value() << " " << hash_value() << std::endl;
                    std::cout << msg->target_hash_value() << std::endl;
                    send_message(m_network, m_fingers[idx - 1], msg);
                    break;
                }
            }
        }

        std::optional<std::shared_ptr<FutureReply>> response;
        while((response = pop_response()).has_value()) {
            auto reply = response.value();
            log().log("Received response: " + ((ChordJumpMessageResponse*)reply->response()->message())->m_content);
            // network->reply(reply->response(), new ChordJumpMessageResponse("test"));
        }

        if(i % 100 == 0 && !m_sentForBuild) {
            build_finger_table(network);
        }
        i++;

        usleep(100000);
    }
}
