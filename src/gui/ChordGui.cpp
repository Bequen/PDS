#include "ChordGui.hpp"
#include "imgui.h"
#include <cfloat>

void ChordGui::draw() {
    ImGui::Begin("Chord");

    int k = 5;

    auto map = m_network->map();

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    static int headIdx = 0;
    std::vector<const char*> items;
    for(auto& chord : map) {
        items.push_back(chord.first.c_str());
    }

    if(ImGui::Combo("Head", &headIdx, items.data(), items.size())) {
        head = items[headIdx];
    }

    if (ImGui::BeginTable("Chords", 4, flags))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Hash value");
        ImGui::TableSetupColumn("Next");
        ImGui::TableHeadersRow();

        for (auto node : map)
        {
            ImGui::PushID(node.first.c_str());
            auto chord = std::dynamic_pointer_cast<Chord>(node.second);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(chord->address.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%i", chord->hash_value());

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(chord->get_next_address().c_str());

            ImGui::TableSetColumnIndex(3);
            if(ImGui::Button("Select")) {
                m_opened.push_back(std::pair(true, chord.get()));
                std::cout << "Done" << std::endl;
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (ImGui::TreeNode("Create node"))
    {
        static char* buf = new char[256];
        ImGui::InputText("Address", buf, 255);
        ImGui::InputInt("Hash value", &createChordForm.hash_value);

        if(ImGui::Button("Create")) {
            m_network->add_node(buf, std::dynamic_pointer_cast<Node>(std::make_shared<Chord>(k, createChordForm.hash_value)));
            createChordForm = CreateChordForm();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Add to chord"))
    {
        static int32_t headIdx = 0;
        static int32_t nodeIdx = 0;
        ImGui::Combo("Head", &headIdx, items.data(), items.size());
        ImGui::Combo("Node", &nodeIdx, items.data(), items.size());

        if(ImGui::Button("Append")) {
            auto node = ((Chord*)m_network->get_node(nullptr, items[nodeIdx]));

            m_network->send_message(items[nodeIdx], items[headIdx], new ChordAddNodeMessage(items[nodeIdx], node->hash_value()));
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Send chord message"))
    {
        static uint32_t mesg_idx = 0;
        static int32_t head_idx = 0;

        static char* head_ip = new char[256];
        static char* content = new char[256];

        static int32_t hash = 1;
        ImGui::Combo("Head", &head_idx, items.data(), items.size());
        ImGui::InputInt("Hash", &hash);
        ImGui::InputText("Content", content, 255);

        if(ImGui::Button("Append")) {
            m_network->send_message("0.0.0.0", items[head_idx], new ChordJumpMessage(hash, content));
        }

        ImGui::TreePop();
    }

    ImGui::End();

    for(int i = 0; i < m_opened.size(); i++) {
        auto chord = m_opened[i];

        ImGui::PushID(chord.second->address.c_str());
        if(ImGui::Begin(chord.second->address.c_str(), &chord.first)) {
            ImGui::Text("Hash value: %i", chord.second->hash_value());
            ImGui::Text("Next: %s", chord.second->get_next_address().c_str());
            ImGui::Text("Fingers:");

            static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

            if (ImGui::BeginTable("Fingers", 3, flags))
            {
                // Display headers so we can inspect their interaction with borders
                // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them now. See other sections for details)
                ImGui::TableSetupColumn("Finger");
                ImGui::TableSetupColumn("Hash value");
                ImGui::TableSetupColumn("Address");
                ImGui::TableHeadersRow();

                int i = 1;
                chord.second->get_fingers();
                for(auto finger : chord.second->get_fingers()) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%i", i);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%i", (i + chord.second->hash_value()) % chord.second->max());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(finger.c_str());
                    i *= 2;
                }

                ImGui::EndTable();
            }
        }

        m_node_gui.draw();

        ImGui::End();

        if(!chord.first) {
            m_opened.erase(m_opened.begin() + i);
        }

        ImGui::PopID();
    }
}
