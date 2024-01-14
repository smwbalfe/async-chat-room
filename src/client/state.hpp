//
// Created by shriller44 on 12/29/23.
//

#ifndef CHAT_STATE_HPP
#define CHAT_STATE_HPP

#include <memory>
#include <utility>
#include <unordered_set>
#include "client/gui/gui_include.hpp"
#include "event_queue.hpp"
#include "client/gui/client_gui.hpp"

namespace state {

    inline std::mutex state_mutex;

    // read , write and gui thread.
    struct global_state {
        std::vector<std::tuple<std::string, std::string>> message_history_;
        std::vector<std::string> online_users_;
    };



    struct gui_state {

        explicit gui_state(const std::vector<std::string>& in) {
            for (const auto& item : in) {
                inputs.insert({ item, "" });
            }
            client_ = std::make_unique<client>();
        }

        // gui , write threads.
        std::map<std::string, std::string> inputs;

        // gui and write threads
        std::string message;

        // read write and gui threads
        std::unique_ptr<client> client_;

        // gui thread only
        gui::client_gui::view currentView { gui::client_gui::view::menu };

        std::string local_username_{"anoymous"};

        std::time_t last_recv;
        event_queue writer_queue_;
        event_queue gui_queue_;
    };

    using state_pair = std::pair<gui_state, global_state>;
    using state_ptr = std::shared_ptr<state_pair>;

    inline auto setup_state(){
        return std::make_shared<state_pair>(std::vector<std::string>{ "text", "username"}, global_state{});
    }
}

#endif //CHAT_STATE_HPP
