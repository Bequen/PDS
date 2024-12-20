#pragma once

class Node;

class NodeGui {
    Node *m_node;

public:
    NodeGui(Node *node);
    ~NodeGui() {}

    void draw();

    void draw_log();
};
