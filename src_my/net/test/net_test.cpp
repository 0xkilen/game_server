#include "net/server.hpp"
#include "net/client.hpp"
#include "net/stream.hpp"
#include "net/conn.hpp"
#include "net/log.hpp"
#include "utils/service_thread.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <condition_variable>
#include <mutex>

namespace nora {
        namespace test {

                class mock_user {
                public:
                        mock_user(const string& name) : name_(name), st_(make_shared<service_thread>("nettest-mockuser")), error_(false) {
                                st_->start();
                                TLOG << name_ << " created";
                        }
                        void stop() {
                                if (conn_) {
                                        conn_->cancel();
                                        conn_.reset();
                                }
                                st_->stop();
                        }
                        shared_ptr<service_thread> get_st() {
                                return st_;
                        }
                        void set_conn(const shared_ptr<net::CONN>& conn) {
                                conn_ = conn;
                        }
                        void reset_conn() {
                                conn_.reset();
                        }
                        shared_ptr<net::CONN> get_conn() {
                                return conn_;
                        }
                        void recv_msg(const shared_ptr<net::recvstream>& msg) {
                                *msg >> msg_;
                        }
                        const string& msg() const {
                                return msg_;
                        }
                        void set_error() {
                                if (conn_) {
                                        conn_->cancel();
                                        conn_.reset();
                                }
                                error_ = true;
                        }
                        bool error() const {
                                return error_;
                        }
                private:
                        const string name_;
                        shared_ptr<service_thread> st_;
                        shared_ptr<net::CONN> conn_;
                        string msg_;
                        bool error_;
                };

                class net_test : public CppUnit::TestFixture {
                        CPPUNIT_TEST_SUITE(net_test);
                        CPPUNIT_TEST(test_connect);
                        CPPUNIT_TEST(test_server_send);
                        CPPUNIT_TEST(test_client_send);
                        CPPUNIT_TEST(test_server_disconnect);
                        CPPUNIT_TEST(test_client_disconnect);
                        CPPUNIT_TEST_SUITE_END();
                public:
                        void setUp() override;
                        void tearDown() override;
                        void test_connect();
                        void test_server_send();
                        void test_client_send();
                        void test_server_disconnect();
                        void test_client_disconnect();
                private:
                        shared_ptr<net::server<net::CONN>> nsv_;
                        unique_ptr<mock_user> su_;
                        shared_ptr<net::client<net::CONN>> ncl_;
                        unique_ptr<mock_user> cu_;
                };

                CPPUNIT_TEST_SUITE_REGISTRATION(net_test);

                void net_test::setUp() {
                        init_log("net_test", pc::options::ERROR);
                        nsv_ = make_shared<net::server<net::CONN>>("nettest-server");
                        su_ = make_unique<mock_user>("nettest-setver-mockuser");
                        ncl_ = make_shared<net::client<net::CONN>>("nettest-client");
                        cu_ = make_unique<mock_user>("nettest-client-mockuser");

                        mutex sm;
                        condition_variable scv;
                        nsv_->new_conn_cb_ = [this, &sm, &scv] (const auto& c) -> shared_ptr<service_thread> {
                                {
                                        lock_guard<mutex> lk(sm);
                                        su_->set_conn(c);
                                }
                                scv.notify_one();
                                return su_->get_st();
                        };
                        nsv_->msg_cb_ = [this] (const auto& c, const auto& msg) { su_->recv_msg(msg); };
                        nsv_->error_cb_ = [this] (const auto& c) { su_->set_error(); };

                        mutex cm;
                        condition_variable ccv;
                        ncl_->new_conn_cb_ = [this, &cm, &ccv] (const auto& c) -> shared_ptr<service_thread> {
                                {
                                        lock_guard<mutex> lk(cm);
                                        cu_->set_conn(c);
                                }
                                ccv.notify_one();
                                return cu_->get_st();
                        };
                        ncl_->msg_cb_ = [this] (const auto& c, const auto& msg) { cu_->recv_msg(msg); };
                        ncl_->error_cb_ = [this] (const auto& c) { cu_->set_error(); };

                        nsv_->start();
                        ncl_->start();
                        nsv_->listen("127.0.0.1", 6017);
                        this_thread::sleep_for(200ms);
                        ncl_->connect("127.0.0.1", 6017);

                        unique_lock<mutex> slk(sm);
                        scv.wait(slk, [this] { return su_->get_conn(); });
                        unique_lock<mutex> clk(cm);
                        ccv.wait(clk, [this] { return cu_->get_conn(); });
                        this_thread::sleep_for(200ms);
                }

                void net_test::tearDown() {
                        if (cu_) {
                                cu_->stop();
                                cu_.reset();
                        }
                        if (su_) {
                                su_->stop();
                                su_.reset();
                        }
                        if (nsv_) {
                                nsv_->stop();
                                nsv_.reset();
                        }
                        if (ncl_) {
                                ncl_->stop();
                                ncl_.reset();
                        }
                }

                void net_test::test_connect() {
                        CPPUNIT_ASSERT(su_->get_conn());
                        CPPUNIT_ASSERT(cu_->get_conn());
                }

                void net_test::test_server_send() {
                        auto ss = make_shared<net::sendstream>();
                        *ss << "hello";
                        nsv_->send(su_->get_conn(), ss);
                        this_thread::sleep_for(200ms);
                        CPPUNIT_ASSERT_EQUAL(string("hello"), cu_->msg());
                }

                void net_test::test_client_send() {
                        auto ss = make_shared<net::sendstream>();
                        *ss << "hello";
                        ncl_->send(cu_->get_conn(), ss);
                        this_thread::sleep_for(200ms);
                        CPPUNIT_ASSERT_EQUAL(string("hello"), su_->msg());
                }

                void net_test::test_server_disconnect() {
                        nsv_->disconnect(su_->get_conn());
                        su_->reset_conn();
                        this_thread::sleep_for(1200ms);
                        CPPUNIT_ASSERT(cu_->error());
                        CPPUNIT_ASSERT_EQUAL(0ul, nsv_->conn_count());
                        CPPUNIT_ASSERT_EQUAL(0ul, ncl_->conn_count());
                }

                void net_test::test_client_disconnect() {
                        ncl_->disconnect(cu_->get_conn());
                        cu_->reset_conn();
                        this_thread::sleep_for(200ms);
                        CPPUNIT_ASSERT(su_->error());
                        CPPUNIT_ASSERT_EQUAL(0ul, nsv_->conn_count());
                        CPPUNIT_ASSERT_EQUAL(0ul, ncl_->conn_count());
                }

        }
}
