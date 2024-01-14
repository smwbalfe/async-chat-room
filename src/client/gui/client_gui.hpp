//
// Created by shriller44 on 12/31/23.
//

#ifndef CHAT_CLIENT_GUI_HPP
#define CHAT_CLIENT_GUI_HPP

#include <fmt/format.h>
#include "client/state.hpp"

namespace gui {

    namespace flags {
        constexpr auto main = ImGuiWindowFlags_NoTitleBar;
    }

    class input_text_wrapper {
    public:
        input_text_wrapper(size_t width, const char *label, bool visible) :
                width_{width},
                label_{label},
                visible_{visible} {}

        void render(std::string &message) {
            if (visible_) {
                ImGui::PushItemWidth(width_);
                ImGui::InputText(label_, &message);
                ImGui::PopItemWidth();
            }
        }

    private:
        size_t width_;
        const char *label_;
        bool visible_;
    };

    class client_gui {
    public:

        enum class view { menu, chat};

        explicit client_gui(state::state_ptr state) : state_{std::move(state)}{}
        void handle_events();
        void render();
        void root_render();
        void render_menu();
        void top_pane();
        void bottom_pane();
        void render_chat();

    private:
        state::state_ptr state_;
        view currentView { view::menu };
    };
}


#endif //CHAT_CLIENT_GUI_HPP
