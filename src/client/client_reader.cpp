//
// Created by shriller44 on 12/31/23.
//

#include "client_reader.hpp"


client_reader::client_reader(state::state_ptr ptr_):
state_ {std::move(ptr_)},
reader_thread_ (&client_reader::read, this)
{}

[[noreturn]] void client_reader::read(){
    for(;;){
        auto& gui_state = state_->first;
        struct visitor {
            client_reader* parent_;
            void operator() (const message& mes) const
            {
                parent_->handle_message(mes);
            }
            void operator()(const chat_state& cs) const{
                parent_->handle_chat_state(cs);
            }
            void operator()(const shriller::netv2::owned_message& om) const {}
            void operator()(std::monostate ms) const {}
        };

        visitor vs { .parent_ = this};

        auto value_ = gui_state.client_->read();

        if (value_.value().index() != 0) {
            std::visit(vs, value_.value());
        }
    }
}

auto client_reader::extract_state(){
    return std::tie(state_->first.local_username_,
                    state_->second.message_history_,
                    state_->second.online_users_);
}

void client_reader::handle_chat_state(const client_reader::chat_state& cs){

    if (!cs.message_history_.empty()) {
        auto latest_message_it = std::ranges::max_element(cs.message_history_, [&](const shriller::netv2::message_info &lhs,
                const shriller::netv2::message_info &rhs) {
            return lhs.timestamp_ < rhs.timestamp_;
        });

        std::time_t latest_message_time = (*latest_message_it).timestamp_;
        std::time_t& last_recv = state_->first.last_recv;

        state_->first.writer_queue_.emplace_back(event_queue::event_info{
            .e_type_ = event_queue::event_type::kTimeUpdate,
            .context_ = std::to_string(latest_message_time)
        });

        if ( latest_message_time == last_recv ) { return; }

        for (const auto& item: cs.message_history_){
            state_->first.gui_queue_.emplace_back(event_queue::event_info{
                    .e_type_ = event_queue::event_type::kReadMessage,
                    .context_vec_ = {item.username_, item.content_}
            });
        }

        last_recv = latest_message_time;
    }

    state_->first.gui_queue_.emplace_back(event_queue::event_info{
            .e_type_ = event_queue::event_type::kOnlineUserUpdate,
            .context_vec_ = cs.online_users_
    });

}

