#include "db/connector.hpp"
#include "db/message.hpp"
#include "utils/log.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <mutex>
#include <string>
#include <condition_variable>
#include <stdlib.h>

using namespace std;

namespace nora {
        namespace test {

                class mock_user {
                public:
                        mock_user() : st_(make_shared<service_thread>("dbtest-mockuser")) {
                                st_->start();
                        }
                        ~mock_user() {
                                st_->stop();
                        }
                        shared_ptr<service_thread> get_st() {
                                return st_;
                        }
                private:
                        shared_ptr<service_thread> st_;
                };

                class db_test : public CppUnit::TestFixture {
                        CPPUNIT_TEST_SUITE(db_test);
                        CPPUNIT_TEST(test_connect_fail);
                        CPPUNIT_TEST(test_connect);
                        CPPUNIT_TEST(test_queries);
                        CPPUNIT_TEST_SUITE_END();
                public:
                        void setUp() override;
                        void test_connect_fail();
                        void test_connect();
                        void test_queries();
                };

                CPPUNIT_TEST_SUITE_REGISTRATION(db_test);

                void db_test::setUp() {
                        init_log("db_test", pc::options::TRACE);

                        system("/usr/bin/mysql -unora -p1234 < sqls/test_db.sql");
                        system("/usr/bin/mysql -unora -p1234 test < sqls/test_table.sql");
                }

                void db_test::test_connect_fail() {
                        db::connector conn("localhost", "nouser", "nopasswd", "nodatabase");
                        CPPUNIT_ASSERT_THROW(conn.start(), sql::SQLException);
                }

                void db_test::test_connect() {
                        db::connector conn("localhost", "root", "1234", "test");
                }

                void db_test::test_queries() {
                        db::connector conn("localhost", "root", "1234", "test");
                        conn.start();

                        mock_user user;
                        mutex m;
                        condition_variable cv;
                        bool done = false;

                        auto msg1 = make_shared<db::message>("add_user",
                                                             db::message::req_type::rt_insert,
                                                             [] (const shared_ptr<db::message>& msg) {
                                                             });
                        msg1->push_param(string("nora"));
                        conn.push_message(msg1, user.get_st());

                        auto msg2 = make_shared<db::message>("find_user",
                                                             db::message::req_type::rt_select,
                                                             [] (const shared_ptr<db::message>& msg) {
                                                                     DLOG << " find user done";
                                                                     CPPUNIT_ASSERT_EQUAL(size_t(1), msg->results().size());
                                                                     CPPUNIT_ASSERT_EQUAL(size_t(1), msg->results().front().size());
                                                                     string out_name;
                                                                     CPPUNIT_ASSERT_NO_THROW(out_name = boost::any_cast<string>(msg->results().front().front()));
                                                                     CPPUNIT_ASSERT_EQUAL(string("nora"), out_name);
                                                             });
                        msg2->push_param(string("nora"));
                        conn.push_message(msg2, user.get_st());

                        auto msg3 = make_shared<db::message>("delete_user",
                                                             db::message::req_type::rt_delete,
                                                             [] (const shared_ptr<db::message>& msg) {
                                                                     DLOG << " delete user done";
                                                             });
                        msg3->push_param(string("nora"));
                        conn.push_message(msg3, user.get_st());

                        auto msg4 = make_shared<db::message>("find_user",
                                                             db::message::req_type::rt_select,
                                                             [&done, &m, &cv] (const shared_ptr<db::message>& msg) {
                                                                     {
                                                                             DLOG << " find user done";
                                                                             lock_guard<mutex> lk(m);
                                                                             done = true;
                                                                     }
                                                                     cv.notify_one();
                                                             });
                        msg4->push_param(string("nora"));
                        conn.push_message(msg4, user.get_st());

                        unique_lock<mutex> lk(m);
                        cv.wait(lk, [&done] { return done; });

                        CPPUNIT_ASSERT_EQUAL(msg4->results().size(), size_t(0));
                }

        }
}
