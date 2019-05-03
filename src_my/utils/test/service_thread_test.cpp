#include "utils/service_thread.hpp"
#include "utils/exception.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <condition_variable>

using namespace chrono;

namespace nora {
namespace test {

        class service_thread_test : public CppUnit::TestFixture {
                CPPUNIT_TEST_SUITE(service_thread_test);
                CPPUNIT_TEST(test_add_timer_by_tp);
                CPPUNIT_TEST(test_add_timer_by_du);
                CPPUNIT_TEST(test_async_call);
                CPPUNIT_TEST(test_multi_async_call_line_up);
                CPPUNIT_TEST_SUITE_END();
        public:
                void setUp() override;
                void tearDown() override;
                void test_add_timer_by_tp();
                void test_add_timer_by_du();
                void test_async_call();
                void test_multi_async_call_line_up();
        private:
                shared_ptr<service_thread> st_;
        };

        CPPUNIT_TEST_SUITE_REGISTRATION(service_thread_test);

        void service_thread_test::setUp() {
                st_ = make_shared<service_thread>("servicethreadtest");
                st_->start();
        }

        void service_thread_test::tearDown() {
                st_->stop();
                st_.reset();
        }

        void service_thread_test::test_add_timer_by_tp() {
                mutex m;
                condition_variable cv;

                auto tp = system_clock::now() + 30ms;
                system_clock::time_point expired_tp;
                ADD_TIMER(
                        st_,
                        ([&m, &cv, &expired_tp] (auto canceled, const auto& timer) {
                                if (!canceled) {
                                        {
                                                lock_guard<mutex> lk(m);
                                                expired_tp = system_clock::now();
                                        }
                                        cv.notify_one();
                                }
                        }),
                        tp);

                unique_lock<mutex> lk(m);
                cv.wait(lk, [&expired_tp] { return expired_tp != system_clock::time_point(); });

                auto error = duration_cast<milliseconds>(expired_tp - tp).count();
                CPPUNIT_ASSERT(error < 20);
        }

        void service_thread_test::test_add_timer_by_du() {
                mutex m;
                condition_variable cv;

                auto start_tp = system_clock::now();
                milliseconds expired_du(0);
                auto du = 25ms;
                ADD_TIMER(
                        st_,
                        ([&m, &cv, &expired_du, start_tp = start_tp] (auto canceled, const auto& timer) {
                                if (!canceled) {
                                        {
                                                lock_guard<mutex> lk(m);
                                                expired_du = duration_cast<milliseconds>(system_clock::now() - start_tp);
                                        }
                                        cv.notify_one();
                                }
                        }),
                        du);

                unique_lock<mutex> lk(m);
                cv.wait(lk, [&expired_du] { return expired_du != 0ms; });

                if (expired_du > du) {
                        CPPUNIT_ASSERT(expired_du - du < 20ms);
                } else {
                        CPPUNIT_ASSERT(du - expired_du < 20ms);
                }
        }

        void service_thread_test::test_async_call() {
                mutex m;
                condition_variable cv;

                thread::id called_thread_id;
                st_->async_call(
                        [&called_thread_id, &cv, &m] () {
                                {
                                        lock_guard<mutex> lk(m);
                                        called_thread_id = this_thread::get_id();
                                }
                                cv.notify_one();
                        });

                unique_lock<mutex> lk(m);
                cv.wait(lk, [&called_thread_id] { return called_thread_id != thread::id(); });

                CPPUNIT_ASSERT(called_thread_id != this_thread::get_id());
                CPPUNIT_ASSERT(called_thread_id == st_->get_thread_id());
        }

        void service_thread_test::test_multi_async_call_line_up() {
                mutex m;
                condition_variable cv;

                bool done = false;
                vector<int> v;
                for (int i = 0; i < 100; ++i) {
                        st_->async_call(
                                [&v, i] {
                                        v.push_back(i);
                                });
                }
                st_->async_call(
                        [&m, &cv, &done] {
                                {
                                        lock_guard<mutex> lk(m);
                                        done = true;
                                }
                                cv.notify_one();
                        });
                unique_lock<mutex> lk(m);
                cv.wait(lk, [&done] { return done; });

                CPPUNIT_ASSERT_EQUAL(100ul, v.size());
                for (size_t i = 0; i < v.size(); ++i) {
                        CPPUNIT_ASSERT_EQUAL((int)i, v.at(i));
                }
        }

}
}
