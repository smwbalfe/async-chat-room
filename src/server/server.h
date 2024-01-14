#include <iostream>
#include <thread>
#include <fstream>
#include <ranges>
#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <fmt/format.h>

#include <boost/asio.hpp>
#include <s_connection.h>
#include <s_tsqueue.h>



class server {
public:

    using parsed_message = std::tuple<std::string, shriller::netv2::message::message_type, uint16_t>;
    using message_info = shriller::netv2::message_info;
    using chat_log = std::vector<message_info>;
    using owned_message = shriller::netv2::owned_message;
    using connection_ptr = std::unique_ptr<shriller::netv2::connection>;

    struct user_connection_info {
        std::string username_;
        connection_ptr connection_;
        std::time_t last_recv_ {0};
    };

    using user_connection_map = std::map <uint16_t, user_connection_info>;

    explicit server(uint16_t port);
    server(const server& other) = delete;
    server& operator=(const server& other) = delete;
    server(server&& other) = delete;
    server& operator=(server&& other) = delete;
    ~server();

    void start_server() noexcept;
    bool read() noexcept;
    void send_state() noexcept;
//    void error_checker() noexcept;

private:

    void server_broadcast(const std::string& message, const std::string& sender);
    parsed_message parse_incoming(const shriller::netv2::message_variant& request);
    void accept_connections();
    void add_user(uint16_t port, boost::asio::ip::tcp::socket& socket);
    void connect_to_user(uint16_t client_port);
    void disconnect_user(uint16_t owner);
    void first_connect(uint16_t owner, std::string& content);
    void timestamp_update(uint16_t owner, std::time_t latest_time);
    std::string& get_username(const uint16_t& port);
    std::vector<server::message_info> new_message_subvec(const server::user_connection_info& con_info);

private:
    boost::asio::io_context context_;
    std::thread context_thread_;
    boost::asio::ip::tcp::acceptor asio_acceptor_;
    chat_log message_history_;
    user_connection_map online_users_;
    shriller::netv2::safequeue message_in_queue_;
};

