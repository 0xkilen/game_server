#pragma once

#include "utils/log.hpp"
#include "utils/service_thread.hpp"
#include <chrono>
#include <memory>
#include <boost/asio.hpp>

using namespace std;
using namespace chrono;
namespace ba = boost::asio;

namespace nora {

        class event_base {
        public:
                virtual ~event_base() {}
        };

        template <typename T>
        class state_base {
        public:
                virtual void on_in(const shared_ptr<T>& owner) = 0;
                virtual state_base *on_event(const shared_ptr<T>& owner, event_base *event) = 0;
                virtual state_base *on_timeout(const shared_ptr<T>& owner) { return nullptr; }
                virtual void on_out(const shared_ptr<T>& owner) = 0;
                virtual system_clock::duration timeout() {
                        return system_clock::duration::zero();
                }
                virtual ~state_base() {}
        };

        template <typename T>
        class state_machine : public enable_shared_from_this<state_machine<T>> {
        public:
                state_machine(const shared_ptr<T>& owner, const shared_ptr<service_thread>& st) : owner_(owner), st_(st) {
                }
                void start(state_base<T> *init_state) {
                        in_state(init_state);
                }
                void stop() {
                        if (stop_) {
                                return;
                        }
                        stop_ = true;
                        if (timeout_timer_) {
                                timeout_timer_->cancel();
                                timeout_timer_.reset();
                        }
                }
                void on_event(event_base *event) {
                        auto owner = owner_.lock();
                        if (owner) {
                                auto *new_state = state_->on_event(owner, event);
                                if (new_state && new_state != state_) {
                                        change_state(new_state);
                                }
                        }
                }
                void reset_timeout(const system_clock::duration& du) {
                        if (timeout_timer_) {
                                timeout_timer_->cancel();
                                timeout_timer_.reset();
                        }
                        if (du != system_clock::duration::zero()) {
                                timeout_timer_ = ADD_TIMER(
                                        st_,
                                        ([this, self = this->shared_from_this()] (auto canceled, const auto& timer) {
                                                if (timeout_timer_ == timer) {
                                                        timeout_timer_.reset();
                                                }
                                                if (!canceled && !stop_) {
                                                        this->on_timeout();
                                                }
                                        }),
                                        du);
                        }
                }
                state_base<T> *get_state() const {
                        return state_;
                }
        private:

                void in_state(state_base<T> *new_state) {
                        auto owner = owner_.lock();
                        if (owner) {
                                state_ = new_state;
                                state_->on_in(owner);
                                reset_timeout(state_->timeout());
                        }
                }

                void on_timeout() {
                        auto owner = owner_.lock();
                        if (owner) {
                                auto *new_state = state_->on_timeout(owner);
                                if (new_state && new_state != state_) {
                                        change_state(new_state);
                                }
                        }
                }

                void out_state() {
                        auto owner = owner_.lock();
                        if (owner) {
                                state_->on_out(owner);
                        }
                }

                void change_state(state_base<T> *new_state) {
                        out_state();
                        in_state(new_state);
                }
        private:
                weak_ptr<T> owner_;
                state_base<T> *state_;
                shared_ptr<service_thread> st_;
                shared_ptr<timer_type> timeout_timer_;
                bool stop_ = false;
        };

}
