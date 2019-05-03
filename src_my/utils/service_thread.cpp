#include "service_thread.hpp"
#include "utils/exception.hpp"
#include "utils/time_utils.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"

using namespace std;
using namespace chrono;

namespace nora {

        mutex service_thread::timer_counts_lock_;
        map<string, int> service_thread::timer_counts_;

        service_thread::service_thread(const string& name) :
                name_(name),
                isw_(make_unique<ba::io_service::work>(is_)),
                stop_(false) {
        }

        void service_thread::start() {
                t_ = make_unique<thread>(
                        [this, self = shared_from_this()] {
                                ILOG << "thread for " << *this;
                                while (!stop_.load()) {
                                        run();
                                }
                        });
        }

        void service_thread::run() {
                tid_ = this_thread::get_id();
                while (!stop_.load()) {
                        try {
                                is_.run();
                        } catch (const std::exception& e) {
                                ELOG << *this << " got exception: " << e.what();
                                continue;
                        }
                        break;
                }
        }

        void service_thread::stop() {
                if (stop_.load()) {
                        return;
                }
                stop_.store(true);

                isw_.reset();
                is_.stop();
                if (t_) {
                        if (!check_in_thread()) {
                                if (t_->joinable()) {
                                        t_->join();
                                        t_.reset();
                                }
                        } else {
                                t_->detach();
                                t_.reset();
                        }
                }
        }

        service_thread::~service_thread() {
                stop();
        }

        void service_thread::async_call(const function<void()>& func) {
                // ASSERT(!is_.stopped());
                is_.post(func);
        }

        void service_thread::call_in_thread(const function<void()>& func) {
                if (check_in_thread()) {
                        func();
                } else {
                        async_call(func);
                }
        }

        shared_ptr<timer_type> service_thread::new_timer() {
                return make_shared<timer_type>(is_);
        }

        shared_ptr<timer_type> service_thread::add_timer(const string& tag, const function<void(bool, const shared_ptr<timer_type>&)>& cb, const system_clock::time_point& tp) {
                if (system_clock::to_time_t(tp) < system_clock::to_time_t(system_clock::now())) {
                        ELOG << "timer expires in the past\n"
                             << "now: " << clock::instance().time_str(system_clock::to_time_t(system_clock::now())) << "\n"
                             << "timer: " << clock::instance().time_str(system_clock::to_time_t(tp));
                        return add_timer(tag, cb, 0s);
                } else {
                        return add_timer(tag, cb, tp - system_clock::now());
                }
        }

        shared_ptr<timer_type> service_thread::add_timer(const string& tag, const function<void(bool, const shared_ptr<timer_type>&)>& cb, const system_clock::duration& du) {
                shared_ptr<timer_type> timer = new_timer();
                {
                        lock_guard<mutex> lock(timer_counts_lock_);
                        timer_counts_[tag] += 1;
                }
                timer->expires_from_now(du);
                timer->async_wait(
                        [this, self = shared_from_this(), tag, timer, cb] (const auto& ec) {
                                ASSERT(this->check_in_thread());
                                {
                                        lock_guard<mutex> lock(timer_counts_lock_);
                                        timer_counts_[tag] -= 1;
                                        if (timer_counts_[tag] == 0) {
                                                timer_counts_.erase(tag);
                                        }
                                }
                                if (!ec) {
                                        cb(false, timer);
                                } else if (ec == ba::error::operation_aborted) {
                                        cb(true, timer);
                                } else {
                                        ELOG << ec.message();
                                }
                        });

                return timer;
        }

        map<string, int> service_thread::timer_counts() {
                lock_guard<mutex> lock(timer_counts_lock_);
                return timer_counts_;
        }

        string service_thread::to_string() const {
                return name_;
        }

        ostream& operator<<(ostream& os, const service_thread& st) {
                return os << st.name_;
        }

}
