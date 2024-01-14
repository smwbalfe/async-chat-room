//
// Created by shriller44 on 12/31/23.
//

#ifndef CHAT_EVENT_QUEUE_HPP
#define CHAT_EVENT_QUEUE_HPP

#include <memory>
#include <utility>
#include <mutex>
#include <deque>
#include <vector>

class event_queue {
public:

    enum class event_type { kWriteMessage, kTimeUpdate, kReadMessage, kMakeConnection, kOnlineUserUpdate};

    struct event_info {
        event_type e_type_;
        std::string context_;
        std::vector<std::string> context_vec_;
    };

    const event_info& last() {
        std::scoped_lock l{ queue_mux };
        return que.back();
    }

    const event_info& first() {
        std::scoped_lock l{ queue_mux };
        return que.front();
    }

    size_t size() {
        std::scoped_lock l{ queue_mux };
        return que.size();
    }

    void wipe() {
        std::scoped_lock l{ queue_mux };
        que.clear();
    }

    void emplace_back(const event_info& newItem) {
        std::scoped_lock l{ queue_mux };
        que.emplace_back(newItem);
    }

    void emplace_front(const event_info& newItem) {
        std::scoped_lock l{ queue_mux };
        que.emplace_front(newItem);
    }

    event_info fetch_back() {

        std::scoped_lock l{ queue_mux };
        auto t = std::move(que.back());
        que.pop_back();
        return t;
    }

    event_info fetch_front() {
        std::scoped_lock l{ queue_mux };
        auto t = std::move(que.front());
        que.pop_front();
        return t;
    }

    bool is_empty() {
        std::scoped_lock l{ queue_mux };
        return que.empty();
    }

private:
    std::deque<event_info> que;
    std::mutex queue_mux;
};


#endif //CHAT_EVENT_QUEUE_HPP
