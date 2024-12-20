#pragma once

#include <set>
#include <vector>
#include <memory>

class Raft;
class Network;

class RaftGui {
    std::vector<std::shared_ptr<Raft>> m_peers;

    std::set<std::string> m_selected;

    Network *m_network;

public:
    RaftGui(Network *network, std::vector<std::shared_ptr<Raft>> peers);

    void draw();
};
