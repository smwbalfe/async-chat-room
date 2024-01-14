#pragma once

#include "netinclude.h"
#include "s_packet.h"


    namespace shriller::netv2 {

		class safequeue {

		public:
			const message_variant & last() {
		
				std::scoped_lock l{ muxQueue };
				return que.back();
			}

			const message_variant& first() {
				std::scoped_lock l{ muxQueue };
				return que.front();
			}

			size_t size() {
				std::scoped_lock l{ muxQueue };
				return que.size();
			}

			void wipe() {
				std::scoped_lock l{ muxQueue };
				que.clear();
			}

			void addBack(const message_variant& newItem) {
			
				std::scoped_lock l{ muxQueue };
				que.emplace_back(newItem);
			}

			void addFront(const message_variant& newItem) {
				std::scoped_lock l{ muxQueue };
				que.emplace_front(newItem);
			}

            message_variant fetchBack() {
			
				std::scoped_lock l{ muxQueue };
				auto t = std::move(que.back());
				que.pop_back();
				return t;
			}

            message_variant fetchFront() {
			
				std::scoped_lock l{ muxQueue };
				auto t = std::move(que.front());
				que.pop_front();
				return t;
			}

			bool isEmpty() {
				std::scoped_lock l{ muxQueue };
				return que.empty();
			}

		private:
			std::deque<message_variant> que;
			std::mutex muxQueue;
			std::condition_variable c;

		};

	}
