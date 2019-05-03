#pragma once

#include "utils/common.hpp"
#include <thread>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>

using namespace std;
using namespace chrono;
namespace ba = boost::asio;
namespace bs = boost::system;

#define ADD_TIMER(st, cb, tp_or_du) st->add_timer(string(__FILE__) + ':' + to_string(__LINE__), cb, tp_or_du);

namespace nora {

        class service_thread : public enable_shared_from_this<service_thread>,
                               private boost::noncopyable {
        public:
                service_thread(const string& name);
                void start();
                void run();
                void stop();
                ~service_thread();
                ba::io_service& get_service();
                thread::id get_thread_id() const;
                shared_ptr<timer_type> add_timer(const string& tag, const function<void(bool, const shared_ptr<timer_type>&)>& cb, const system_clock::time_point& tp);
                shared_ptr<timer_type> add_timer(const string& tag, const function<void(bool, const shared_ptr<timer_type>&)>& cb, const system_clock::duration& du);
                void async_call(const function<void()>& func);
                void call_in_thread(const function<void()>& func);
                bool check_in_thread() const;
                static map<string, int> timer_counts();
                string to_string() const;
                friend ostream& operator<<(ostream& os, const service_thread& st);
        private:
                shared_ptr<timer_type> new_timer();

                const string name_;
                ba::io_service is_;
                unique_ptr<ba::io_service::work> isw_;
                unique_ptr<thread> t_;
                thread::id tid_;
                atomic<bool> stop_;
                static mutex timer_counts_lock_;
                static map<string, int> timer_counts_;
        };

        inline ba::io_service& service_thread::get_service() {
                return is_;
        }

        inline thread::id service_thread::get_thread_id() const {
                return tid_;
        }

        inline bool service_thread::check_in_thread() const {
                return tid_  == this_thread::get_id();
        }

}
