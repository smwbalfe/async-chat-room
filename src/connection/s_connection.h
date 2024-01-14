
#include "netinclude.h"
#include "s_tsqueue.h"
#include <ranges>



namespace shriller {
	namespace netv2 {

		constexpr size_t HEADER_LENGTH = 64;
			
		enum class owner {
			server,
			client
		};

		enum MessageTypes {
			empty,
			chatState,
			normalMessage,
			messageOwned
		};

		enum QueueTypes {
			state_,
			message_,
		};

		struct info {
			std::size_t inboundDataSize;
			int index;
		};

		using MessageTypeVar = std::variant <std::monostate, message, chat_state> ;


		class connection : public std::enable_shared_from_this<connection> {
		
		public:

			connection(owner ownert, boost::asio::ip::tcp::socket socket, boost::asio::io_context& ctx, safequeue& msgin) :
				socket_{ std::move(socket) }, ctx_{ ctx }, msgin_{ msgin } {
				owner_type_ = ownert;
			}

			template<typename ... lambdas>
			struct overload : lambdas... {
				using lambdas::operator()...;
			};

		
			void serializeOutbound() {
			
				message_variant msg = msgout_.first();
				
				std::ostringstream arcstrm;
				boost::archive::text_oarchive arc(arcstrm);
				
				overload visitor{[&]<typename type>(type data) {
					if constexpr (!std::is_same_v<type, std::monostate>) {
						arc << data;
					}
				}};

				std::visit(visitor, msg);

				outbound_body_ = arcstrm.str();

			}
			void HandleError(const std::error_code& ec) {


				switch (ec.value()) {
				case 10061 :
					std::cout << "failure to connect to host" << std::endl;
					//socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
					socket_.close();
					break;
				case 10054:
					std::cout << "client has left" << std::endl;
                        error_code_ = 10054;
					break;
				default :
					std::cout << "error in handle error: " << ec.message() << std::endl;
				}
			}


			void send(message_variant && msg) {
				
				boost::asio::post(ctx_, [this, msg]() {

					const bool hasAnItem = !msgout_.isEmpty();

					msgout_.addBack(msg);

					if (!hasAnItem) {
						write_index_ = static_cast<int>(msg.index());
						WriteHeader();
					} 
				});
				
			}

			void read_header() {
				boost::asio::async_read(socket_, boost::asio::buffer(tempHeaderInbound_.data(), tempHeaderInbound_.size()),
					[this](const std::error_code ec, std::size_t bytes){
							if (!ec){
								info i {0, 0};
                                std::string temp_header {tempHeaderInbound_.data(), HEADER_LENGTH};
                                std::erase_if(temp_header, isspace);
                                std::vector<std::string> v1;
                                std::ranges::for_each(std::views::split(temp_header, ','), [&](const auto& s){
                                    v1.emplace_back(s.begin(), s.end());
                                });
                                std::stringstream stream{ v1[0] };
                                stream >> std::hex >> i.inboundDataSize;
                                i.index = std::stoi(v1[1]);
								tempBodyInbound_.resize(i.inboundDataSize);
								read_index_ = i.index;
                                read_body();
							}
					}
				);

			}

			void read_body() {


				boost::asio::async_read(socket_, boost::asio::buffer(tempBodyInbound_.data(), tempBodyInbound_.size()),
					[this](const std::error_code ec , std::size_t bytes){
						std::string arcdata{tempBodyInbound_.data(), tempBodyInbound_.size()};
						std::istringstream arcstrm{ arcdata };
						boost::archive::text_iarchive arc { arcstrm };

						switch (read_index_) {
						case 0:
                            temp_msg_in = std::monostate{};
							break;
						case 1: {
							shriller::netv2::chat_state cs{};
							arc >> cs;
                            temp_msg_in = cs;
							break;
						}
						case 2: {
							shriller::netv2::message mes{};
							arc >> mes;
                            temp_msg_in = mes;
							break;
						}
						default: break;
						}

						addToQueue();
						 
					}
				);
			}

			void WriteHeader() {
				serializeOutbound();

				std::ostringstream header;

				/* header: set number of spaces to be the size of header
				 *
				 * encode the size of the body and type of message being sent denoted by the writeIndex_
				 */
				header << std::setw(HEADER_LENGTH) << std::hex << std::format("{:x},{}", outbound_body_.size(), write_index_);
				
				headerOutbound_ = header.str();

				
				async_write(socket_, boost::asio::buffer(headerOutbound_),
					[this](const std::error_code ec, std::size_t bytes) {
						if (!ec) {						
							WriteBody();
						}
						else {
							HandleError(ec);
						}
					}
				);
			}

			void WriteBody() {

				async_write(socket_, boost::asio::buffer(outbound_body_),
					[this](const std::error_code ec, std::size_t bytes) {
						msgout_.fetchFront();
						if (!msgout_.isEmpty()) {
							WriteHeader();
							
						}
					}
				
				);
			}

			void addToQueue() {
				message_variant m_var;
				overload visitor{ [&]<typename type>(type data) {
					if constexpr (std::is_same_v<type, message>) {
						if (owner_type_ == owner::server) {
							m_var.emplace<owned_message>(std::get<message_>(temp_msg_in), socket_.remote_endpoint().port());
							return;
						}
					}
					m_var = data;
				} };
				std::visit(visitor, temp_msg_in);

				msgin_.addBack(m_var);

				read_header();
			}


            void connectToServer(const boost::asio::ip::tcp::resolver::results_type& eps) {
                boost::asio::async_connect(socket_, eps,
                                           [this](const std::error_code ec, const boost::asio::ip::tcp::endpoint endpoint)
                                           {
                                               if (!ec)
                                               {

                                                   read_header();
                                               }
                                               else {

                                                   HandleError(ec);
                                               }
                                           });
            }

			[[nodiscard]] auto remoteEndpoint() const{ return socket_.remote_endpoint(); }
			[[nodiscard]] int read_error() const{ return error_code_; }

            bool is_open() const {return socket_.is_open();}

            bool connectToClient() {
                if (owner_type_ == owner::server) {
                    if (socket_.is_open()) {
                        read_header();
                        return true;
                    }
                }
                return false;
            }
			
		private:

            // the socket associated with this connection
			boost::asio::ip::tcp::socket socket_;

            // the asio context running the async operations
			boost::asio::io_context& ctx_;

            // the output queue that messages to be sent are placed into (dequeued by WriteBody / WriteHeader)
			safequeue msgout_;

            // the input queue that receives messages (queued by ReadHeader / ReadBody)
			safequeue& msgin_;
            std::array<char, HEADER_LENGTH> tempHeaderInbound_;
			std::vector<char> tempBodyInbound_;
			MessageTypeVar temp_msg_in;
			std::string outbound_body_;
			owner owner_type_ = owner::server;
			std::string headerOutbound_;
			int write_index_ {};
			int read_index_  {};
            int error_code_{0};
		};

	}
}
