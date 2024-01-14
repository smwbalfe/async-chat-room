#ifndef CHAT_CLIENT_READER_HPP
#define CHAT_CLIENT_READER_HPP

#include <fmt/format.h>
#include "clientmain.h"
#include "state.hpp"

class client_reader {
public:

    using chat_state = shriller::netv2::chat_state;
    using message = shriller::netv2::message;

    explicit client_reader(state::state_ptr ptr_);
    [[noreturn]] void read();

private:
    void handle_message(const message& mes){
        switch (mes.type) {
            case shriller::netv2::message::message_type::kDirectMessage:
                std::cout << "[ " << mes.username << " ]: " << mes.content << std::endl;
                break;
            default:;
        }
    }

    auto extract_state();
    void handle_chat_state(const chat_state& cs);

    std::string last_recv_;
    std::jthread reader_thread_;
    state::state_ptr state_;
};


#endif //CHAT_CLIENT_READER_HPP
