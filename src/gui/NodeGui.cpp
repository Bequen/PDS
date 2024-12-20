#include "NodeGui.hpp"

#include "network/Node.hpp"
#include "imgui.h"
#include <ranges>

NodeGui::NodeGui(Node *node) :
    m_node(node) {

}

void NodeGui::draw() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::BeginChild("MessagesChild", ImVec2(0, 260), ImGuiChildFlags_Borders, window_flags);
    if (ImGui::BeginTable("Messages", 4, flags))
    {
        ImGui::TableSetupColumn("From");
        ImGui::TableSetupColumn("Display");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Is request");

        ImGui::TableHeadersRow();

        //for(auto message : (m_node->received_messages())) {
        for(int i = m_node->received_messages().size() - 1; i >= 0; i--) {
            auto& message = m_node->received_messages()[i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", message.from().c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", message.display().c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%i", message.type());

            ImGui::TableSetColumnIndex(3);
            // ImGui::Checkbox(message.is_response());
        }

        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void NodeGui::draw_log() {
    ImGui::BeginChild("log", ImVec2(0, 0), ImGuiChildFlags_Borders);

    ImGuiListClipper clipper;
    clipper.Begin(m_node->log().size());
    auto& log = m_node->log();
    while (clipper.Step())
    {
        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
        {
            // const char* line_start = buf + LineOffsets[line_no];
            // const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
            ImGui::Text("%s", log.get(line_no).message.c_str());
        }
    }
    clipper.End();

    ImGui::EndChild();
}
