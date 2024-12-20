#pragma once

#include <map>
#include <string>
#include <set>

class Network;

class NodeGui;

class NetworkGui {
private:
    Network *m_network;
    float m_timeScaleBeforePause = 0.0f;
    std::set<std::string> m_selected;

    void draw_nodes();

public:
    NetworkGui(Network* network) : m_network(network) {}

    void draw();
};
