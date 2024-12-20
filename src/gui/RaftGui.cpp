#include "RaftGui.hpp"

#include "gui/NodeGui.hpp"
#include "network/Node.hpp"
#include "imgui.h"

#include "raft/Raft.hpp"
#include "raft/RaftMessage.hpp"
#include <memory>

RaftGui::RaftGui(Network *network, std::vector<std::shared_ptr<Raft>> peers) :
    m_network(network),
    m_peers(peers) {

}

void RaftGui::draw() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;



    ImGui::Text("Cluster");
    ImGui::BeginChild("MessagesBlock", ImVec2(ImGui::GetContentRegionAvail().x, 260), ImGuiChildFlags_None, window_flags);
    if (ImGui::BeginTable("Messages", 5, flags))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("State");
        ImGui::TableSetupColumn("Log index");
        ImGui::TableSetupColumn("Has voted");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        int i = 0;
        for (auto& node : m_peers)
        {
            ImGui::PushID(node->address.c_str());
            ImGui::TableNextRow();
            if(node->raft_state() == LEADER) {
                ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f)); // Flat or Gradient?
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0 + 1, row_bg_color);
            }
            if(node->state() == STOPPED) {
                ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.65f)); // Flat or Gradient?
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0 + 1, row_bg_color);
            }
            ImGui::TableSetColumnIndex(0);

            bool isSelected = m_selected.count(node->address) > 0;
            bool wasSelected = isSelected;
            ImGui::Selectable(node->address.c_str(), &isSelected, ImGuiSelectableFlags_SpanAllColumns);
            if(isSelected && !wasSelected) {
                m_selected.insert(node->address);
            } else if(!isSelected && wasSelected) {
                m_selected.erase(node->address);
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%i", node->raft_state());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%i", node->log_idx());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", node->has_voted() ? "yes" : "no");

            ImGui::TableSetColumnIndex(4);
            // if(node->state() == STOPPED) {
            //     if(ImGui::Button("Start")) {
            //         node->set_state(RUNNING);
            //     }
            // } else {
            //     if(ImGui::Button("Stop")) {
            //         node->set_state(STOPPED);
            //     }
            // }

            // ImGui::SameLine();
            // if(ImGui::Button("Time out")) {
            //     node->time_out();
            // }

            i++;
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();


    for(auto& selected : m_selected) {
        auto node = (Raft*)(m_network->get_node(nullptr, selected));
        if(node == nullptr) {
            continue;
        }

        ImGui::Begin(selected.c_str());

        static char* buf = new char[256];
        ImGui::InputText("Content", buf, 256);
        if(ImGui::Button("Send Request")) {
            m_network->send_message("0.0.0.0", selected, new RaftRequest(strdup(buf)));
        }

        if(ImGui::Button("Time out")) {
            node->time_out();
        }

        NodeGui(node).draw();

        ImGui::Text("Parameters");
        if(ImGui::BeginTable("Parameters", 2, flags)) {
            ImGui::TableSetupColumn("Key");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Log index");
            ImGui::TableNextColumn();
            ImGui::Text("%u", node->log_idx());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Voted");
            ImGui::TableNextColumn();
            ImGui::Text("%s", node->has_voted() ? "yes" : "no");

            ImGui::EndTable();
        }

        if(node->raft_state() == LEADER) {
            ImGui::Text("Leader");


            ImGui::Text("Peers");
            if(ImGui::BeginTable("Peers", 4, flags)) {
                ImGui::TableSetupColumn("Peer");
                ImGui::TableSetupColumn("Last Index");
                ImGui::TableSetupColumn("Match Index");
                ImGui::TableSetupColumn("Peer Term Index");
                ImGui::TableHeadersRow();

                for(auto& peer : node->peers()) {
                    if(peer == node->address) {
                        continue;
                    }

                    auto reply = node->response_for(peer);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", peer.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", node->next_idx_for(peer));
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", node->match_idx_for(peer));
                    ImGui::TableNextColumn();
                    if(true) {
                        auto success = node->last_success_for(peer);
                        ImGui::Text("%s", success ? "success" : "failure");
                    } else {
                        ImGui::Text("Waiting");
                    }

                }
                ImGui::EndTable();
            }
        }

        ImGui::Text("Log");
        ImGui::BeginChild("RaftLogChild", ImVec2(0, 260), ImGuiChildFlags_Borders, window_flags);
        if(ImGui::BeginTable("Raft Log", 3, flags)) {
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Term");
            ImGui::TableSetupColumn("Command");
            ImGui::TableHeadersRow();

            for(auto& entry : node->log().entries()) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%u", entry.idx());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", entry.term());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", entry.command().c_str());
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();

        ImGui::Text("Votes");
        ImGui::Text("For: %i", node->votes_for());
        ImGui::Text("Against: %i", node->votes_againts());

        ImGui::BeginChild("RaftVotes", ImVec2(0, 260), ImGuiChildFlags_Borders, window_flags);
        if(ImGui::BeginTable("Waiting for votes", 2, flags)) {
            ImGui::TableSetupColumn("Address");
            ImGui::TableHeadersRow();

            for(auto& vote : node->future_votes()) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(vote->target().c_str());
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();

        NodeGui(node).draw_log();

        ImGui::End();
    }
}
