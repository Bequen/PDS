#pragma once

#include "gui/NodeGui.hpp"
#include "network/Network.hpp"
#include "chord/Chord.hpp"
#include <memory>
#include <cstring>

struct CreateChordForm {
    char* address;
    int hash_value;

    CreateChordForm() :
        address(new char[256]),
        hash_value(0) {
            std::memset(address, 0, 256);
        }
};

class ChordGui {
private:
    Network* m_network;
    std::shared_ptr<Chord> m_head;

    CreateChordForm createChordForm;

    std::vector<std::pair<bool, Chord*>> m_opened;
    std::string head;
    uint32_t head_idx;

    NodeGui m_node_gui;

public:
    void set_head_idx(uint32_t idx) {
        head_idx = idx;
    }

    ChordGui(Network* network, std::shared_ptr<Chord> head) :
        m_network(network),
        m_head(head),
        createChordForm(),
        m_opened(),
        m_node_gui(head.get()){

    }

    void draw();
};
