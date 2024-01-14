//
// Created by shriller44 on 12/31/23.
//

#include "client_gui.hpp"

void gui::client_gui::render() {
    handle_events();
    root_render();
}

void gui::client_gui::handle_events(){
    auto& g_queue = state_->first.gui_queue_;
    while (!g_queue.is_empty()){
        auto event = g_queue.fetch_front();
        switch (event.e_type_){
            case event_queue::event_type::kReadMessage: {
                state_->second.message_history_.emplace_back(std::tuple{event.context_vec_[0], event.context_vec_[1]});
                break;
            }
            case event_queue::event_type::kOnlineUserUpdate: {
                state_->second.online_users_ = std::move(event.context_vec_);
            }
        }
    }
}

void gui::client_gui::root_render() {
    auto &gui_state = state_->first;
    switch (gui_state.currentView) {
        case view::menu: {
            render_menu();
            break;
        }
        case view::chat:
            render_chat();
    }
}
void gui::client_gui::render_menu() {

    auto &gui_state = state_->first;

    ImGui::Begin("Main Menu", nullptr);
    input_text_wrapper username{150, "Username", true};
    username.render(gui_state.inputs.find("username")->second);
    if (ImGui::Button("Connect")) {
        if (gui_state.inputs.find("username")->second.empty()){
            fmt::print("you must enter a username\n");
        } else {
            gui_state.writer_queue_.emplace_back(event_queue::event_info{
                    .e_type_ = event_queue::event_type::kMakeConnection,
                    .context_ = gui_state.inputs.find("username")->second
            });
            gui_state.currentView = view::chat;
        }
    }
    ImGui::End();

}
void gui::client_gui::top_pane() {

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(61, 60, 59, 255).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255).Value);

    ImGui::BeginChild(
            "ChildL",
            ImVec2(ImGui::GetContentRegionAvail().x * 0.60f, ImGui::GetContentRegionAvail().y * 0.80f),
            false,
            window_flags
    );

    for (const auto &[username, message]: state_->second.message_history_) {
        ImGui::Text("%s", fmt::format("[ {} ] {}", username, message).c_str());
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::BeginChild("ChildTwo",
                      ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.80f), false,
                      window_flags);

    ImGui::Text("Users Online");

    for (const auto &n: state_->second.online_users_) {
        ImGui::Text("%s", n.c_str());
    }
    ImGui::EndChild();
}
void gui::client_gui::bottom_pane() {

    ImGui::BeginChild("BottomWindow",
                      ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y),
                      false);

    std::string& message_box_entry = state_->first.inputs.find("text")->second;

    input_text_wrapper message_box{150, "##", true};
    message_box.render(message_box_entry);
    ImGui::SameLine();

    if (ImGui::Button("Send Message")) {
        state_->first.writer_queue_.emplace_back(event_queue::event_info {
            .e_type_ = event_queue::event_type::kWriteMessage,
            .context_ = message_box_entry
        });
        message_box_entry.clear();
    };

    ImGui::PushStyleColor(ImGuiCol_Button, ImColor(255, 0, 0, 255).Value);
    if (ImGui::Button("Leave")) {
        state_->first.client_->send({state_->first.local_username_, shriller::netv2::message::message_type::kDisconnect});
        std::exit(1);
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();
}
void gui::client_gui::render_chat() {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y});
    ImGui::Begin("Main Window", nullptr, flags::main);
    top_pane();
    bottom_pane();
    ImGui::End();
}