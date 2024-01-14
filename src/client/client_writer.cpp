//
// Created by shriller44 on 12/31/23.
//

#include "client_writer.hpp"


client_writer::client_writer(state::state_ptr ptr_):
state_ {std::move(ptr_)},
writer_thread_ (&client_writer::write, this)
{}

auto client_writer::extract_state(){
    return std::tie(
            state_->first.inputs,
            state_->first.client_,
            state_->first.local_username_,
            state_->second.message_history_
    );
}

[[noreturn]] void client_writer::write() {
    for (;;){
        handle_event();
    }
}

void client_writer::handle_event() {
    auto& writer_queue = state_->first.writer_queue_;
    if (!writer_queue.is_empty()){
        auto event = writer_queue.fetch_front();
        switch (event.e_type_){
            case event_queue::event_type::kWriteMessage: {
                fmt::print("sending: {}\n", event.context_);
                state_->first.client_->send({
                    .content = event.context_,
                    .type = shriller::netv2::message::message_type::kBroadcastMessage,
                   .username =  ""});
                break;
            }
            case event_queue::event_type::kTimeUpdate: {
               state_->first.client_->send(shriller::netv2::message {
                   .content = event.context_,
                   .type = shriller::netv2::message::message_type::kTimestampUpdate,
                   .username = "timestamp",
                   .timestamp = shriller::netv2::unix_time(),
               });
               break;
            }
            case event_queue::event_type::kMakeConnection: {
                state_->first.local_username_ = event.context_;
                state_->first.client_->connect_to_server("127.0.0.1", 60000);
                state_->first.client_->send(shriller::netv2::message{
                        .content = event.context_,
                        .type = shriller::netv2::message::message_type::kFirstConnect,
                        .username = "connection",
                        .timestamp = shriller::netv2::unix_time(),
                });
            }
        }
    }
}