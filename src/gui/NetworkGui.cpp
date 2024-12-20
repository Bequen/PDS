#include "NetworkGui.hpp"

#include "gui/NodeGui.hpp"
#include "network/Network.hpp"
#include "imgui.h"

void
NetworkGui::draw_nodes() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("Messages", 2, flags))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableHeadersRow();

        for (auto& node : m_network->map())
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            bool isSelected = m_selected.count(node.first) > 0;
            bool wasSelected = isSelected;
            ImGui::Selectable(node.first.c_str(), &isSelected, ImGuiSelectableFlags_SpanAllColumns);
            if(isSelected && !wasSelected) {
                m_selected.insert(node.first);
            } else if(!isSelected && wasSelected) {
                m_selected.erase(node.first);
            }
        }
        ImGui::EndTable();
    }

    for(auto& selected : m_selected) {
        auto node = m_network->get_node(nullptr, selected);
        if(node == nullptr) {
            continue;
        }

        ImGui::Begin(selected.c_str());

        NodeGui(node).draw();

        ImGui::End();
    }
}

void
NetworkGui::draw() {
    ImGui::Begin("Network");

    if(*m_network->timescale_ptr() == 0.0f) {
        if(ImGui::Button("Play")) {
            *m_network->timescale_ptr() = m_timeScaleBeforePause;
        }
    } else {
        if(ImGui::Button("Pause")) {
            m_timeScaleBeforePause = *m_network->timescale_ptr();
            *m_network->timescale_ptr() = 0.0f;
        }
    }
    ImGui::SameLine();

    if(ImGui::SliderFloat("Time speed", m_network->timescale_ptr(), 0.0, 0.1)) {
        m_timeScaleBeforePause = *m_network->timescale_ptr();
    }

    // draw_nodes();

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    if (ImGui::BeginTable("Messages", 4, flags))
    {
        // Display headers so we can inspect their interaction with borders
        // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them now. See other sections for details)
        ImGui::TableSetupColumn("From");
        ImGui::TableSetupColumn("Progress");
        ImGui::TableSetupColumn("Is response");
        ImGui::TableSetupColumn("Target");
        ImGui::TableHeadersRow();

        uint32_t idx = 0;
        for (auto& message : m_network->messages())
        {
            if(message.message() == nullptr) {
                continue;
            }
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(message.from().c_str());

            ImGui::TableSetColumnIndex(1);
            if(idx == 0) {
                ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
            }
            if(message.is_response()) {
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 0, 255));
                ImGui::ProgressBar(message.state(), ImVec2(0.0f, 0.0f), (std::string(message.message()->display())).c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::ProgressBar(message.state(), ImVec2(0.0f, 0.0f), (std::string(message.message()->display())).c_str());
            }

            ImGui::TableSetColumnIndex(2);
            if(message.is_response()) {
                ImGui::Text("Response");
            } else {
                ImGui::Text("Request");
            }

            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(message.target().c_str());
            idx++;
        }
        ImGui::EndTable();
    }

    ImGui::End();
}
