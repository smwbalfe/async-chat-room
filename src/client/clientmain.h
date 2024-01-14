#pragma once

#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <thread>
#include <s_tsqueue.h>
#include <s_connection.h>

class client {
public:

    using message_opt = std::optional<shriller::netv2::message_variant>;

    client();
	~client();
	void connect_to_server(const std::string& host, const std::uint16_t& port);
	void send(shriller::netv2::message) const;
    [[maybe_unused]] bool connected();
    message_opt read();
private:
	std::unique_ptr<shriller::netv2::connection> connection_;
	std::jthread m_thread;
	boost::asio::io_context m_context;
	shriller::netv2::safequeue msgin_;
};
