#pragma once

#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "net/server.hpp"
#include "net/conn.hpp"

namespace ps = proto::ss;

namespace nora {

        template <typename T, typename U = net::CONN>
        class client_process : public proto::processor<client_process<T, U>, ps::base>,
                               public enable_shared_from_this<client_process<T, U>> {
        public:
                client_process(const shared_ptr<service_thread>& st, const shared_ptr<net::server<U>>& nsv, const shared_ptr<U>& c, uint32_t client_id) :
                        st_(st), nsv_(nsv), conn_(c), client_id_(client_id) {
                        client_process_ = make_shared<T>(conn_->name());
                }
                static void static_init(const shared_ptr<service_thread>& st = nullptr) {
                        client_process::register_proto_handler(ps::s2s_register_req::descriptor(), &client_process<T, U>::process_register_req);
                        T::static_init(st);
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
                                                client_process_->process(client_process_.get(), &ss_msg);
                                        }
                                });
                }
                void stop() {
                        st_->async_call(
                                [this, self = this->shared_from_this()] {
                                        if (client_process_) {
                                                client_process_->on_stop();
                                        }
                                        if (stop_cb_ && conn_) {
                                                stop_cb_(conn_, this->shared_from_this());
                                        }
                                        if (nsv_ && conn_) {
                                                nsv_->disconnect(conn_);
                                                nsv_.reset();
                                                conn_.reset();
                                        }
                                        register_cb_ = nullptr;
                                        stop_cb_ = nullptr;
                                });
                }
                friend ostream& operator<<(ostream& os, const client_process<T, U>& cp) {
                        return os << *cp.client_process_;
                }
                int id() const {
                        ASSERT(client_process_);
                        return client_process_->id();
                }
                void send_msg(const ps::base& msg) {
                        ASSERT(st_);
                        st_->async_call(
                                [this, self = this->shared_from_this(), msg] {
                                        TLOG << *this << " send " << msg.DebugString();
                                        auto ss = make_shared<net::sendstream>();
                                        msg.SerializeToOstream(ss.get());
                                        nsv_->send(conn_, ss);
                                });
                }

                function<void(const shared_ptr<U>&)> register_cb_;
                function<void(const shared_ptr<U>&, const shared_ptr<client_process<T, U>>&)> stop_cb_;
        private:
                void process_register_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2s_register_req::srr);
                        ASSERT(client_process_);
                        client_process_->set_id(req.id());
                        client_process_->set_st(st_);

                        register_cb_(conn_);

                        proto::ss::base rsp_msg;
                        auto *rsp = rsp_msg.MutableExtension(ps::s2s_register_rsp::srr);
                        rsp->set_id(client_id_);
                        send_msg(rsp_msg);
                }

                shared_ptr<service_thread> st_;
                shared_ptr<T> client_process_;
                shared_ptr<net::server<U>> nsv_;
                shared_ptr<U> conn_;
                uint32_t client_id_ = 0;
        };

}
