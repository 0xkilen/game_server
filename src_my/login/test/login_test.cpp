#include "net/client.hpp"
#include "net/conn.hpp"
#include "net/stream.hpp"
#include "login/logind.hpp"
#include "proto/processor.hpp"
#include "proto/ss_login.pb.h"
#include "proto/cs_login.pb.h"
#include <memory>
#include <condition_variable>
#include <cppunit/extensions/HelperMacros.h>

namespace ps = proto::ss;

namespace nora {
        namespace test {

                class mock_scene : public enable_shared_from_this<mock_scene> {
                public:
                        mock_scene(const string& name) : st_(make_shared<service_thread>(string("logintest-mockscene-") + name)), name_(name) {
                        }
                        void connect(const string& login_ip, unsigned short login_port,
                                     const string& client_ip, unsigned short client_port) {
                                auto self = shared_from_this();
                                ncl_ = make_shared<net::client<net::CONN>>(name_ + "-netclient");
                                ncl_->new_conn_cb_ = [self] (const auto& c) -> shared_ptr<service_thread> {
                                        {
                                                lock_guard<mutex> lk(self->m_);
                                                self->conn_ = c;
                                        }
                                        self->cv_.notify_one();
                                        return self->st_;
                                };
                                ncl_->msg_cb_ = [self] (const auto& c, const auto& msg) {};
                                ncl_->error_cb_ = [self] (const auto& c) {};

                                st_->start();

                                ncl_->start();
                                ncl_->connect(login_ip, login_port);

                                unique_lock<mutex> lk(m_);
                                cv_.wait(lk, [self] { return self->conn_; });

                                ps::base msg;
                                auto *r = msg.MutableExtension(ps::s2l_register_req::srr);
                                r->set_client_ip(client_ip);
                                r->set_client_port(client_port);

                                auto ss = make_shared<net::sendstream>();
                                msg.SerializeToOstream(ss.get());
                                ncl_->send(conn_, ss);
                        }
                        void sync_status(int client_count) {
                                ps::base msg;
                                auto *sss = msg.MutableExtension(ps::s2l_sync_status::sss);
                                sss->set_client_count(client_count);

                                auto ss = make_shared<net::sendstream>();
                                msg.SerializeToOstream(ss.get());
                                ncl_->send(conn_, ss);
                        }
                        void stop() {
                                if (conn_) {
                                        ncl_->disconnect(conn_);
                                        conn_.reset();
                                }
                                if (ncl_) {
                                        ncl_->stop();
                                        ncl_.reset();
                                }
                                if (st_) {
                                        st_->stop();
                                        st_.reset();
                                }
                        }
                        const string& to_string() const {
                                return name_;
                        }
                        friend ostream& operator<<(ostream& os, const mock_scene& ms);
                private:
                        shared_ptr<net::client<net::CONN>> ncl_;
                        shared_ptr<net::CONN> conn_;
                        shared_ptr<service_thread> st_;
                        mutex m_;
                        condition_variable cv_;
                        const string name_;
                };

                ostream& operator<<(ostream& os, const mock_scene& ms) {
                        return os << ms.to_string();
                }

                class mock_client : public proto::processor<mock_client, proto::cs::base>, public enable_shared_from_this<mock_client> {
                public:
                        mock_client() : st_(make_shared<service_thread>("logintest-mockclient")) {
                                st_->start();
                        }

                        void connect(const string& login_ip, unsigned short login_port) {
                                register_proto_handler(proto::cs::s2c_login_rsp::descriptor(), &mock_client::process_msg_login_rsp);

                                auto self = shared_from_this();
                                ncl_ = make_shared<net::client<net::CONN> >("mockclient-netclient");
                                ncl_->new_conn_cb_ = [self] (const auto& c) -> shared_ptr<service_thread> {
                                        {
                                                lock_guard<mutex> lk(self->m_);
                                                self->conn_ = c;
                                        }
                                        self->cv_.notify_one();
                                        return self->st_;
                                };
                                ncl_->msg_cb_ = [self] (const auto& c, const auto& msg) {
                                        self->process_msg(msg);
                                };
                                ncl_->error_cb_ = [self] (const auto& c) {};
                                ncl_->start();

                                ncl_->connect(login_ip, login_port);

                                unique_lock<mutex> lk(m_);
                                cv_.wait(lk, [self] { return self->conn_; });
                        }
                        void process_msg(const shared_ptr<net::recvstream>& msg) {
                                proto::cs::base cs_msg;
                                ASSERT(cs_msg.ParseFromIstream(msg.get()));
                                process(this, &cs_msg);
                        }
                        void process_msg_login_rsp(const proto::cs::base *msg) {
                                const auto& rsp = msg->GetExtension(proto::cs::s2c_login_rsp::slr);
                                if (rsp.result() == proto::data::REDIRECT) {
                                        // ip_ = rsp.ip();
                                        // port_ = rsp.port();
                                } else {
                                        ELOG << "login failed:\n" << rsp.DebugString();
                                }
                        }
                        void stop() {
                                if (conn_) {
                                        ncl_->disconnect(conn_);
                                        conn_.reset();
                                }
                                if (ncl_) {
                                        ncl_->stop();
                                        ncl_.reset();
                                }
                                if (st_) {
                                        st_->stop();
                                        st_.reset();
                                }
                        }
                        const string& ip() const {
                                return ip_;
                        }
                        unsigned short port() const {
                                return port_;
                        }
                        string to_string() const {
                                return "mockclient";
                        }
                        friend ostream& operator<<(ostream& os, const mock_client& mc);
                private:
                        shared_ptr<net::client<net::CONN>> ncl_;
                        shared_ptr<net::CONN> conn_;
                        shared_ptr<service_thread> st_;
                        string ip_;
                        unsigned short port_;
                        mutex m_;
                        condition_variable cv_;
                };

                ostream& operator<<(ostream& os, const mock_client& mc) {
                        return os << mc.to_string();
                }

                class login_test : public CppUnit::TestFixture {
                        CPPUNIT_TEST_SUITE(login_test);
                        CPPUNIT_TEST(test_client_get_ipport_from_right_scene);
                        CPPUNIT_TEST_SUITE_END();
                public:
                        void setUp() override;
                        void tearDown() override;
                        void test_client_get_ipport_from_right_scene();
                private:
                        shared_ptr<login::logind> lg_;
                        shared_ptr<mock_scene> ms1_;
                        shared_ptr<mock_scene> ms2_;
                        shared_ptr<mock_client> mc_;
                };

                CPPUNIT_TEST_SUITE_REGISTRATION(login_test);

                void login_test::setUp() {
                        init_log("login_test", pc::options::ERROR);

                        lg_ = make_shared<login::logind>(nullptr);
                        lg_->start_client("127.0.0.1", 6017);
                        ms1_ = make_shared<mock_scene>("mockscene-1");
                        ms1_->connect("127.0.0.1", 6018,
                                      "scene-1-ip", 6019);
                        ms2_ = make_shared<mock_scene>("mockscene-2");
                        ms2_->connect("127.0.0.1", 6018,
                                      "scene-2-ip", 6019);
                        this_thread::sleep_for(100ms);
                }

                void login_test::tearDown() {
                        if (lg_) {
                                lg_->stop();
                                lg_.reset();
                        }
                        if (ms1_) {
                                ms1_->stop();
                                ms1_.reset();
                        }
                        if (ms2_) {
                                ms2_->stop();
                                ms2_.reset();
                        }
                        if (mc_) {
                                mc_->stop();
                                mc_.reset();
                        }
                }

                void login_test::test_client_get_ipport_from_right_scene() {
                        ms1_->sync_status(1);
                        ms2_->sync_status(2);
                        this_thread::sleep_for(100ms);

                        mc_ = make_shared<mock_client>();
                        mc_->connect("127.0.0.1", 6017);
                        this_thread::sleep_for(100ms);

                        CPPUNIT_ASSERT_EQUAL(string("scene-1-ip"), mc_->ip());
                        CPPUNIT_ASSERT_EQUAL(6019, (int)mc_->port());

                        ms1_->sync_status(10);
                        this_thread::sleep_for(100ms);
                        mc_->connect("127.0.0.1", 6017);
                        this_thread::sleep_for(100ms);
                        CPPUNIT_ASSERT_EQUAL(string("scene-2-ip"), mc_->ip());
                        CPPUNIT_ASSERT_EQUAL(6019, (int)mc_->port());
                }

        }
}
