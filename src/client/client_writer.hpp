//
// Created by shriller44 on 12/31/23.
//

#ifndef CHAT_CLIENT_WRITER_HPP
#define CHAT_CLIENT_WRITER_HPP

#include <fmt/format.h>
#include "clientmain.h"
#include "state.hpp"

class client_writer {

public:
    explicit client_writer(state::state_ptr ptr_);
    auto extract_state();
    [[noreturn]] void write();
private:
    void handle_event();
    std::jthread writer_thread_;
    state::state_ptr state_;
    shriller::netv2::safequeue event_queue_;
};


#endif //CHAT_CLIENT_WRITER_HPP
