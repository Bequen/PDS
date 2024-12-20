#include <chrono>
#include <cmath>
#include <memory>
#include <thread>

#include "gui/MutexGui.hpp"
#include "gui/RaftGui.hpp"
#include "mutex/Mutex.hpp"
#include "network/Network.hpp"
#include "chord/Chord.hpp"
#include "gui/NetworkGui.hpp"
#include "gui/ChordGui.hpp"
#include "gui/Gui.hpp"
#include "imgui.h"
#include "raft/Raft.hpp"


int chord() {
    int k = 5;

    auto network = Network();
    auto head =  std::dynamic_pointer_cast<Chord>(network.add_node("192.168.0.1", std::dynamic_pointer_cast<Node>(std::make_shared<Chord>(k, 5))));
    auto node1 = network.add_node("192.168.0.2", std::dynamic_pointer_cast<Node>(std::make_shared<Chord>(k, 12)));
    auto node2 = network.add_node("192.168.0.3", std::dynamic_pointer_cast<Node>(std::make_shared<Chord>(k, 18)));
    auto node3 = network.add_node("192.168.0.4", std::dynamic_pointer_cast<Node>(std::make_shared<Chord>(k, 27)));
    auto node4 = network.add_node("192.168.0.5", std::dynamic_pointer_cast<Node>(std::make_shared<Chord>(k, 16)));

    network.send_message("0.0.0.0", "192.168.0.1", new ChordAddNodeMessage("192.168.0.2", 12));
    network.send_message("0.0.0.0", "192.168.0.1", new ChordAddNodeMessage("192.168.0.3", 18));
    network.send_message("0.0.0.0", "192.168.0.1", new ChordAddNodeMessage("192.168.0.4", 27));
    network.send_message("0.0.0.0", "192.168.0.1", new ChordAddNodeMessage("192.168.0.5", 16));


    // network.send_message("192.168.0.2", new ChordGetFingersMessage(5, "192.168.0.1"));


    //auto node = (Chord*)network.get_node(head.get(), "192.168.0.1");
    //node->print_finger_table();

    bool isGui = true;
    if(isGui) {
        Gui gui;
        gui.start();

        NetworkGui networkGui(&network);
        ChordGui chordGui(&network, head);

        while(!gui.should_close()) {
            gui.begin_frame();

            ImGui::ShowDemoWindow();

            networkGui.draw();
            chordGui.draw();
            network.update();

            gui.end_frame();
        }
    } else {
        //*network.timescale_ptr() = 0.5f;
        int i = 0;
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        while(true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            network.update();

            if((end - start) > std::chrono::milliseconds(2000)) {
                start = end;
                std::cout << "Sending" << std::endl;
                network.send_message("0.0.0.0", "192.168.0.1", new ChordJumpMessage(10, "test"));
            }
        }
    }

    return 0;
}

void raft() {
    auto network = Network();
    std::vector<std::string> peers = {
        "192.168.0.1",
        "192.168.0.2",
        "192.168.0.3",
        "192.168.0.4",
        "192.168.0.5",
    };

    std::vector<std::shared_ptr<Raft>> cluster;
    auto node = network.add_node("192.168.0.1", std::dynamic_pointer_cast<Node>(std::make_shared<Raft>(peers)));
    cluster.push_back(std::dynamic_pointer_cast<Raft>(node));
    node = network.add_node("192.168.0.2", std::dynamic_pointer_cast<Node>(std::make_shared<Raft>(peers)));
    cluster.push_back(std::dynamic_pointer_cast<Raft>(node));
    node = network.add_node("192.168.0.3", std::dynamic_pointer_cast<Node>(std::make_shared<Raft>(peers)));
    cluster.push_back(std::dynamic_pointer_cast<Raft>(node));
    node = network.add_node("192.168.0.4", std::dynamic_pointer_cast<Node>(std::make_shared<Raft>(peers)));
    cluster.push_back(std::dynamic_pointer_cast<Raft>(node));
    node = network.add_node("192.168.0.5", std::dynamic_pointer_cast<Node>(std::make_shared<Raft>(peers)));
    cluster.push_back(std::dynamic_pointer_cast<Raft>(node));

    bool isGui = true;
    if(isGui) {
        Gui gui;
        gui.start();

        NetworkGui networkGui(&network);
        RaftGui raftGui(&network, cluster);

        while(!gui.should_close()) {
            gui.begin_frame();

            ImGui::ShowDemoWindow();

            networkGui.draw();
            raftGui.draw();
            network.update();

            gui.end_frame();
        }
    }
}

void mutex() {
    auto network = Network();
    auto node = network.add_node("192.168.0.1", std::dynamic_pointer_cast<Node>(std::make_shared<MutexSheriff>()));
    std::string sheriff = "192.168.0.1";
    auto sheriff_node = std::dynamic_pointer_cast<MutexSheriff>(node);

    std::vector<std::shared_ptr<Mutex>> mutexes;

    node = network.add_node("192.168.0.2", std::dynamic_pointer_cast<Node>(std::make_shared<Mutex>(sheriff)));
    mutexes.push_back(std::dynamic_pointer_cast<Mutex>(node));
    node = network.add_node("192.168.0.3", std::dynamic_pointer_cast<Node>(std::make_shared<Mutex>(sheriff)));
    mutexes.push_back(std::dynamic_pointer_cast<Mutex>(node));
    node = network.add_node("192.168.0.4", std::dynamic_pointer_cast<Node>(std::make_shared<Mutex>(sheriff)));
    mutexes.push_back(std::dynamic_pointer_cast<Mutex>(node));
    node = network.add_node("192.168.0.5", std::dynamic_pointer_cast<Node>(std::make_shared<Mutex>(sheriff)));
    mutexes.push_back(std::dynamic_pointer_cast<Mutex>(node));

    bool isGui = true;
    if(isGui) {
        Gui gui;
        gui.start();

        NetworkGui networkGui(&network);
        MutexGui mutexGui(sheriff_node, mutexes);

        while(!gui.should_close()) {
            gui.begin_frame();

            ImGui::ShowDemoWindow();

            networkGui.draw();
            mutexGui.draw();
            network.update();

            gui.end_frame();
        }
    }
}

int main(int32_t argc, char** argv) {
    if(argc < 2) {
        std::cout << "Please choose algorithm, one of: [raft, chord, mutex]" << std::endl;
        return 1;
    }

    if(!strcmp(argv[1], "raft")) {
        raft();
    } else if(!strcmp(argv[1], "chord")) {
        chord();
    } else if(!strcmp(argv[1], "mutex")) {
        mutex();
    } else {
        std::cout << "Invalid algorithm " << argv[1] << std::endl;
        return 1;
    }


    return 0;
}
