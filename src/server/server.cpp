#include "server.h"

server::server(uint16_t port) : asio_acceptor_{ context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port) }{}

server::~server(){context_thread_.join();}

void server::start_server() noexcept {
    accept_connections();
    context_thread_ = std::thread([this]() {
        context_.run();
    });
}

void server::add_user(uint16_t port, boost::asio::ip::tcp::socket& socket){
    online_users_.insert({ port, user_connection_info("anom_username", std::make_unique<shriller::netv2::connection>(
            shriller::netv2::owner::server,
            std::move(socket),
            context_,
            message_in_queue_
    ))});
}

void server::connect_to_user(uint16_t client_port) {
    online_users_.find(client_port)->second.connection_->connectToClient();
}

void server::accept_connections() {
    std::cout << "listening for connections\n";
    asio_acceptor_.async_accept(
            [this](const std::error_code ec, boost::asio::ip::tcp::socket socket) {
                std::cout << "accepted connection\n";
                auto port = socket.remote_endpoint().port();
                add_user(port, socket);
                connect_to_user(port);
                accept_connections();
            }
    );

}

void server::disconnect_user(uint16_t owner){
    auto& username = get_username(owner);
    fmt::print("[DISCONNECT]: {}\n", username);
    online_users_.erase(owner);
    std::stringstream ss;
    ss << std::format("{} has left the server", username);
    server_broadcast(ss.str(), "server");
}

void server::first_connect(uint16_t owner, std::string& set_username){
    get_username(owner) = set_username;
    std::stringstream ss;
    ss << std::format("{} has joined the server", set_username);
    server_broadcast(ss.str(), "server");
}

void server::timestamp_update(uint16_t owner, std::time_t latest_time){
//    fmt::print("[{}] last recv: {}\n", get_username(owner), latest_time);

    auto max = std::ranges::max_element(message_history_, [&](const shriller::netv2::message_info &lhs,
                                                      const shriller::netv2::message_info &rhs) {
        return lhs.timestamp_ < rhs.timestamp_;
    });
//    fmt::print("actual last recv in server state:{}\n", (*max).timestamp_);
    online_users_.find(owner)->second.last_recv_ = latest_time;
}

server::parsed_message server::parse_incoming(const shriller::netv2::message_variant& request){
    auto owner = std::get<owned_message>(request).port;
    auto message = std::get<owned_message>(request).msg;
    return std::make_tuple(message.content, message.type,owner);
}

bool server::read() noexcept {
    if (!message_in_queue_.isEmpty()) {

        auto [content, type, remote_port] = parse_incoming(message_in_queue_.fetchFront());

        fmt::print("remote_port: {} | content: {} \n", remote_port, content);

        switch (type) {
            case shriller::netv2::message::message_type::kBroadcastMessage:
                server_broadcast(content, get_username(remote_port));
                break;
            case shriller::netv2::message::message_type::kDisconnect: {
                disconnect_user(remote_port);
                break;
            }
            case shriller::netv2::message::message_type::kFirstConnect: {
                first_connect(remote_port, content);
                break;
            }
            case shriller::netv2::message::message_type::kTimestampUpdate: {
                timestamp_update(remote_port, static_cast<time_t>(std::stoll(content)));
            }
            default: ;
        }
    }
    return true;
}

void server::server_broadcast(const std::string& message_content, const std::string& sender) {

    auto time = shriller::netv2::unix_time();

    fmt::print("message: {}\n", message_content);
    message_history_.emplace_back(message_info(sender, message_content, "",  time));

    for (auto& [client, conn] : online_users_) {

        if (get_username(client) != sender) {

            conn.connection_->send(
                    shriller::netv2::message{
                            .content = message_content,
                            .type = shriller::netv2::message::message_type::kDirectMessage,
                            .username = sender,
                            .timestamp = time
                    }
            );
        }
    }
}

std::vector<server::message_info> server::new_message_subvec(const server::user_connection_info& con_info){
    std::time_t last_recv = con_info.last_recv_;
    auto message_comparator = [&]( std::time_t latest, const server::message_info& info){
        return latest < info.timestamp_;
    };
    auto begin_message = std::upper_bound(message_history_.begin(),
                                          message_history_.end(),
                                          last_recv,
                                          message_comparator);

    return {begin_message, message_history_.end()};
}
void server::send_state() noexcept {

    std::this_thread::sleep_for(std::chrono::milliseconds(60));

    std::vector<std::string> online_user_state;
    online_user_state.reserve(online_users_.size());
    for (auto& [port, conn] : online_users_) {
        online_user_state.emplace_back(conn.username_);
    }
    std::ranges::sort(message_history_, [](const server::message_info& a, const server::message_info& b){
        return a.timestamp_ < b.timestamp_;
    });

    for (auto& [port , conn] : online_users_) {
        conn.connection_->send( shriller::netv2::chat_state{ new_message_subvec(conn), online_user_state } );
    }
}

//void server::error_checker() noexcept {
//
//    for (auto it = online_users_.begin(); it != online_users_.end();)
//    {
//        fmt::print("deref\n");
//        auto& [port, connection] = *it;
//        fmt::print("after deref\n");
//
//        const auto error = connection.connection_->readError();
//        auto username = get_username(port);
//
//        switch (error) {
//            case 10054: {
//                fmt::print("erased: {}\n", port);
//                online_users_.erase(it++);
//                std::stringstream ss;
//                ss << std::format("{} has left the server", username);
////                message_history_.emplace_back(message_info("server", ss.str(), username));
//                server_broadcast(ss.str(), username);
//                break;
//            }
//            default:
//                ++it;
//        }
//    }
//}

std::string& server::get_username(const uint16_t& port) {
    return online_users_.find(port)->second.username_;
}