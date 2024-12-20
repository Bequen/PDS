#include "MutexGui.hpp"
#include "imgui.h"
#include "mutex/Mutex.hpp"

MutexGui::MutexGui(std::shared_ptr<MutexSheriff> sheriff,
    std::vector<std::shared_ptr<Mutex>> mutexes) :
    m_sheriff(sheriff),
    m_mutexes(mutexes) {

}

void MutexGui::draw() {
    ImGui::Begin("Mutexes");

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

    if (ImGui::BeginTable("Queue", 1, flags))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableHeadersRow();

        int i = 0;
        for (auto& node : m_sheriff->queue())
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", node.c_str());
        }
        ImGui::EndTable();
    }

    if (ImGui::BeginTable("Messages", 3, flags))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Amount Of Work");
        ImGui::TableSetupColumn("Action");
        ImGui::TableHeadersRow();

        int i = 0;
        for (auto& node : m_mutexes)
        {
            ImGui::PushID(node->address.c_str());
            ImGui::TableNextRow();
            if(m_sheriff->worker().has_value() && m_sheriff->worker() == node->address) {
                ImU32 row_bg_color = 0;
                if(node->is_working()) {
                    row_bg_color = ImGui::GetColorU32(ImVec4(0.4f, 0.7f, 0.3f, 0.65f)); // Flat or Gradient?
                } else {
                    row_bg_color = ImGui::GetColorU32(ImVec4(0.4f, 0.3f, 0.7f, 0.65f)); // Flat or Gradient?
                }
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0 + 1, row_bg_color);
            }

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", node->address.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%u", node->remaining_work());

            ImGui::TableSetColumnIndex(2);
            if(ImGui::Button("Work")) {
                node->request();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::End();
}
