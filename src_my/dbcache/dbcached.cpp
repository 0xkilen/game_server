#include "dbcached.hpp"
#include "utils/process_utils.hpp"

namespace nora {
        namespace dbcache {

                dbcached::dbcached(const shared_ptr<service_thread>& st) {
                        if (!st_) {
                                st_ = make_shared<service_thread>("dbcached");
                                st_->start();
                        }
                        scene_ = make_shared<scene>("dbcache-scene", st_);
                }

                void dbcached::start() {
                        register_signals();
                        auto ptt = PTTS_GET_COPY(options, 1u);
                        scene_->stop_cb_ = [this] {
                                stop();
                        };
                        scene_->start(ptt.scened_dbcached_ipport().ip(), (unsigned short)ptt.scened_dbcached_ipport().port());

                        add_instance_count_timer();
                }

                void dbcached::add_instance_count_timer() {
                        ADD_TIMER(
                                st_,
                                ([this] (auto canceled, const auto& timer) {
                                        if (!canceled) {
                                                instance_counter::instance().print();
                                                this->add_instance_count_timer();
                                        }
                                }),
                                5s);
                }

                void dbcached::register_signals() {
                        if (!signals_) {
                                signals_ = make_unique<ba::signal_set>(st_->get_service());
                                signals_->add(SIGBUS);
                                signals_->add(SIGSEGV);
                                signals_->add(SIGABRT);
                                signals_->add(SIGFPE);
                                signals_->add(SIGTERM);
                                signals_->add(SIGINT);
                                auto ptt = PTTS_GET_COPY(options, 1u);
                                if (ptt.minicore()) {
                                        signals_->add(SIGBUS);
                                        signals_->add(SIGSEGV);
                                        signals_->add(SIGABRT);
                                        signals_->add(SIGFPE);
                                }
                        }

                        signals_->async_wait(
                                [this] (auto ec, auto sig) {
                                        if (ec) {
                                                return;
                                        }

                                        if (sig == SIGTERM || sig == SIGINT) {
                                                DBCACHE_ILOG << "received SIGTERM or SIGINT, stop";
                                                signals_->cancel();
                                                if (scene_) {
                                                        scene_->try_stop();
                                                } else {
                                                        this->stop();
                                                }
                                        } else if (sig == SIGBUS || sig == SIGSEGV || sig == SIGABRT || sig == SIGFPE) {
                                                write_stacktrace_to_file("dbcached");
                                                exit(0);
                                        } else {
                                                this->register_signals();
                                        }
                                });
                }

                void dbcached::stop() {
                        exit(0);
                }

        }
}
