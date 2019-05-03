#include "utils/state_machine.hpp"
#include <string>
#include <memory>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace chrono;

namespace nora {
namespace test {

        class owner;

        class event_a : public event_base {
        public:
                string to_string() {
                        return string("event_a");
                }
        };

        class event_b : public event_base {
        public:
                string to_string() {
                        return string("event_b");
                }
        };

        class event_c : public event_base {
        public:
                string to_string() {
                        return string("event_c");
                }
        };

        class state_a : public state_base<owner> {
        public:
                static state_base<owner> *instance() {
                        static state_a inst;
                        return &inst;
                }
                void on_in(const shared_ptr<owner>& owner) {}
                state_base<owner> *on_event(const shared_ptr<owner>& owner, event_base *event);
                state_base<owner> *on_timeout(const shared_ptr<owner>& owner) { return nullptr; }
                void on_out(const shared_ptr<owner>& owner) {}
        };

        class state_b : public state_base<owner> {
        public:
                static state_base<owner> *instance() {
                        static state_b inst;
                        return &inst;
                }
                void on_in(const shared_ptr<owner>& owner) {}
                state_base<owner> *on_event(const shared_ptr<owner>& owner, event_base *event);
                state_base<owner> *on_timeout(const shared_ptr<owner>& owner) { return state_a::instance(); }
                void on_out(const shared_ptr<owner>& owner) {}
                system_clock::duration timeout() {
                        return 100ms;
                }
        };

        class state_c : public state_base<owner> {
        public:
                static state_base<owner> *instance() {
                        static state_c inst;
                        return &inst;
                }
                void on_in(const shared_ptr<owner>& owner) {}
                state_base<owner> *on_event(const shared_ptr<owner>& owner, event_base *event);
                state_base<owner> *on_timeout(const shared_ptr<owner>& owner) { return nullptr; }
                void on_out(const shared_ptr<owner>& owner) {}
        };

        class owner : public enable_shared_from_this<owner> {
        public:
                owner() : st_(make_shared<service_thread>("statemachinetest-owner")) {
                }
                void init() {
                        sm_ = make_shared<state_machine<owner>>(shared_from_this(), st_);
                        sm_->start(state_a::instance());
                        st_->start();
                }
                ~owner() {
                        st_->stop();
                }
                void on_event(event_base *event) {
                        sm_->on_event(event);
                }
                state_base<owner> *get_state() const {
                        return sm_->get_state();
                }
                void reset_timeout(const system_clock::duration& du) {
                        sm_->reset_timeout(du);
                }
        private:
                shared_ptr<service_thread> st_;
                shared_ptr<state_machine<owner>> sm_;
        };

        class state_machine_test : public CppUnit::TestFixture {
                CPPUNIT_TEST_SUITE(state_machine_test);
                CPPUNIT_TEST(test_on_event);
                CPPUNIT_TEST(test_on_timeout);
                CPPUNIT_TEST(test_reset_timeout);
                CPPUNIT_TEST_SUITE_END();
        public:
                state_machine_test() {}
                state_machine_test(const state_machine_test& other)=delete;
                state_machine_test& operator=(const state_machine_test& other)=delete;
                void test_on_event();
                void test_on_timeout();
                void test_reset_timeout();
        };

        state_base<owner> *state_a::on_event(const shared_ptr<owner>& owner, event_base *event) {
                if (dynamic_cast<const event_b *>(event)) {
                        return state_b::instance();
                } else if (dynamic_cast<const event_c *>(event)) {
                        return state_c::instance();
                }
                return nullptr;
        }

        state_base<owner> *state_b::on_event(const shared_ptr<owner>& owner, event_base *event) {
                if (dynamic_cast<const event_a *>(event)) {
                        return state_a::instance();
                } else if (dynamic_cast<const event_c *>(event)) {
                        return state_c::instance();
                }
                return nullptr;
        }

        state_base<owner> *state_c::on_event(const shared_ptr<owner>& owner, event_base *event) {
                if (dynamic_cast<const event_a *>(event)) {
                        return state_a::instance();
                } else if (dynamic_cast<const event_b *>(event)) {
                        return state_b::instance();
                }
                return nullptr;
        }

        CPPUNIT_TEST_SUITE_REGISTRATION(state_machine_test);

        void state_machine_test::test_on_event() {
                auto o = make_shared<owner>();
                o->init();

                event_a a;
                o->on_event(&a);
                CPPUNIT_ASSERT_EQUAL(state_a::instance(), o->get_state());
                event_b b;
                o->on_event(&b);
                CPPUNIT_ASSERT_EQUAL(state_b::instance(), o->get_state());
                event_c c;
                o->on_event(&c);
                CPPUNIT_ASSERT_EQUAL(state_c::instance(), o->get_state());
        }

        void state_machine_test::test_on_timeout() {
                auto o = make_shared<owner>();
                o->init();

                event_b b;
                o->on_event(&b);

                this_thread::sleep_for(120ms);
                CPPUNIT_ASSERT_EQUAL(state_a::instance(), o->get_state());
        }

        void state_machine_test::test_reset_timeout() {
                auto o = make_shared<owner>();
                o->init();

                event_b b;
                o->on_event(&b);

                this_thread::sleep_for(80ms);
                o->reset_timeout(80ms);
                this_thread::sleep_for(60ms);
                CPPUNIT_ASSERT_EQUAL(state_b::instance(), o->get_state());
                this_thread::sleep_for(40ms);
                CPPUNIT_ASSERT_EQUAL(state_a::instance(), o->get_state());
        }
}
}
