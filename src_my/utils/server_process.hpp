#pragma once

#include "proto/processor.hpp"
#include "proto/ss_base.pb.h"
#include "net/client.hpp"
#include "net/conn.hpp"

namespace ps = proto::ss;

namespace nora {

        template <typename T, typename U = net::CONN>
        class server_process : public proto::processor<server_process<T, U>, ps::base>,
                               public enable_shared_from_this<server_process<T, U>> {
        public:
                server_process(const shared_ptr<service_thread>& st, const shared_ptr<net::client<U>>& ncl, const shared_ptr<U>& c) :
                        st_(st), ncl_(ncl), conn_(c) {
                        server_process_ = make_shared<T>(conn_->name());
                }
                static void static_init() {
                        server_process::register_proto_handler(ps::s2s_register_rsp::descriptor(), &server_process<T, U>::process_register_rsp);
                        T::static_init();
                }
                void process_msg(const shared_ptr<net::recvstream>& msg) {
                        st_->async_call(
                                [this, self = this->shared_from_this(), msg] {
                                        ps::base ss_msg;
                                        ASSERT(ss_msg.ParseFromIstream(msg.get()));
                                        TLOG << *this << " process msg\n" << ss_msg.DebugString();
                                        if (this->has_handler(this, &ss_msg)) {
                                                this->process(this, &ss_msg);
                                        } else {
                                                server_process_->process(server_process_.get(), &ss_msg);
                                        }
                                });
                }
                void stop() {
                        st_->async_call(
                                [this, self = this->shared_from_this()] {
                                        if (stop_cb_ && conn_) {
                                                stop_cb_(conn_, this->shared_from_this());
                                        }
                                        if (ncl_ && conn_) {
                                                ncl_->disconnect(conn_);
                                                conn_.reset();
                                                ncl_.reset();
                                        }
                                        stop_cb_ = nullptr;
                                        register_done_cb_ = nullptr;
                                });
                }
                void send_msg(const ps::base& msg) {
                        ASSERT(st_);
                        st_->async_call(
                                [this, self = this->shared_from_this(), msg] {
                                        TLOG << *this << " send " << msg.DebugString();
                                        auto ss = make_shared<net::sendstream>();
                                        msg.SerializeToOstream(ss.get());
                                        ncl_->send(conn_, ss);
                                });
                }
                friend ostream& operator<<(ostream& os, const server_process<T, U>& cp) {
                        return os << *cp.server_process_;
                }

                int id() const {
                        ASSERT(server_process_);
                        return server_process_->id();
                }

                const shared_ptr<U> conn() const {
                        ASSERT(conn_);
                        return conn_;
                }

                function<void()> register_done_cb_;
                function<void(const shared_ptr<U>&, const shared_ptr<server_process<T, U>>&)> stop_cb_;
        private:
                void process_register_rsp(const ps::base *msg) {
                        ASSERT(st_->check_in_thread());
                        ILOG << *this << " register done";

                        const auto& rsp = msg->GetExtension(ps::s2s_register_rsp::srr);
                        server_process_->set_id(rsp.id());

                        ASSERT(register_done_cb_);
                        register_done_cb_();

                        server_process_->on_register_done();
                }
                shared_ptr<service_thread> st_;
                shared_ptr<net::client<U>> ncl_;
                shared_ptr<U> conn_;
                shared_ptr<T> server_process_;
        };

}
