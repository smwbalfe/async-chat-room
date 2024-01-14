#include "netinclude.h"


	namespace shriller::netv2
	{


        inline auto unix_time(){
            return duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

        inline std::string unix_time_to_string(std::time_t unixTime) {
            std::tm *timeInfo = std::localtime(&unixTime);
            char buffer[80];
            std::strftime(buffer, sizeof(buffer), "%H:%M:%S", timeInfo);
            return buffer;
        }

        struct message_info {
            std::string username_;
            std::string content_;
            std::string ignore_;
            std::time_t timestamp_;

            template<typename Archive>
            void serialize(Archive& ar, const unsigned int version) {
                ar & username_;
                ar & content_;
                ar & ignore_;
                ar & timestamp_;
            }
        };

		struct chat_state {
			std::vector<message_info> message_history_;
			std::vector<std::string> online_users_;

			template<typename Archive>
			void serialize(Archive& ar, const unsigned int version) {
				ar & message_history_;
				ar & online_users_;
			}

		};

		struct message
		{

            enum class message_type {
                kBroadcastMessage,
                kDirectMessage,
                kDisconnect,
                kFirstConnect,
                kTimestampUpdate
            };

            std::string content;
            message_type type;
			std::string username;
            std::time_t timestamp;

		
			template<typename Archive>
			void serialize(Archive& ar, const unsigned int version)
			{
				ar& type;
				ar& content;
				ar& username;
			}
		};

        struct owned_message {
            message msg;
            uint16_t port;
            template<typename Archive>
            void serialize(Archive& ar, const unsigned int version)
            {
                ar& msg;
                ar& port;
            }

        };

		using message_variant = std::variant<std::monostate, chat_state, message, owned_message>;
	}

