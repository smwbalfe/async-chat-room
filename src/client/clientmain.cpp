
#include "clientmain.h"


client::client() {}

client::~client() {
	m_thread.join();

}

void client::connect_to_server(const std::string &host, const std::uint16_t &port) {

	connection_ = std::make_unique<shriller::netv2::connection>(shriller::netv2::owner::client,
                                                                boost::asio::ip::tcp::socket{ m_context }, m_context, msgin_);

	boost::asio::ip::tcp::resolver resolver { m_context };
	boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

	connection_->connectToServer(endpoints);

	m_thread = std::jthread([this]() { m_context.run(); });
}

void client::send(shriller::netv2::message mes) const {
	connection_->send(mes);	
}

[[maybe_unused]] bool client::connected() {
	return connection_->is_open();
}

client::message_opt client::read() {
    client::message_opt  retopt;
	if (!msgin_.isEmpty()) {
		return msgin_.fetchFront();
	}
	return retopt.value_or(std::monostate{});
}

