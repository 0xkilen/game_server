#pragma once

#include "service_thread.hpp"
#include <memory>

using namespace std;

namespace nora {

        class server_process_base {
        public:
                virtual ~server_process_base() {
                }
                virtual uint32_t id() const {
                        return id_;
                }
                virtual void set_id(uint32_t id) {
                        id_ = id;
                }
                void set_st(const shared_ptr<service_thread>& st) {
                        st_ = st;
                }
                virtual void on_register_done() {
                }
        protected:
                shared_ptr<service_thread> st_;
        private:
                uint32_t id_ = 0;
        };

}
