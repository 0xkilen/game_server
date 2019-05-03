#include "scene.hpp"
#include "config/utils.hpp"
#include "log.hpp"
#include "config/options_ptts.hpp"
#include "proto/ss_common_resource.pb.h"
#include "utils/client_process_mgr.hpp"
#include "config/recharge_ptts.hpp"

namespace nora {
        namespace common_resource {

                using scene_mgr = client_process_mgr<scene>;

                shared_ptr<db::connector> scene::db_;
                map<string, scene::fanli> scene::username2fanli_;
                map<string, scene::back> scene::come_backs_;
                function<void()> scene::done_cb_;

                string scene::base_name() {
                        return "scene";
                }

                scene::scene(const string& conn_name) : name_("common_resource-scene-" + conn_name) {
                }

                void scene::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::s2cr_lock_fanli_req::descriptor(), &scene::process_lock_fanli_req);
                        register_proto_handler(ps::s2cr_remove_fanli::descriptor(), &scene::process_remove_fanli);
                        register_proto_handler(ps::s2cr_lock_come_back_req::descriptor(), &scene::process_lock_come_back_req);
                        register_proto_handler(ps::s2cr_remove_come_back::descriptor(), &scene::process_remove_come_back);

                        PTTS_LOAD(recharge);
                        auto ptt = PTTS_GET_COPY(options, 1);
                        db_ = make_shared<db::connector>(ptt.common_resource_info().db().ipport(),
                                                         ptt.common_resource_info().db().user(),
                                                         ptt.common_resource_info().db().password(),
                                                         ptt.common_resource_info().db().database());
                        db_->start();

                        ASSERT(st);
                        auto shandang_db_msg = make_shared<db::message>("load_shandang_fanli",
                                                               db::message::req_type::rt_select,
                                                               [] (const shared_ptr<db::message>& msg) {
                                                                       auto& results = msg->results();
                                                                       for (const auto& i : results) {
                                                                               ASSERT(i.size() == 2);
                                                                               auto username = boost::any_cast<string>(i[0]);
                                                                               auto recharge_id = boost::any_cast<string>(i[1]);
                                                                               for (const auto& i : PTTS_GET_ALL(recharge)) {
                                                                                       if (i.second.yunying_id() == recharge_id) {
                                                                                               if (username2fanli_.count(username) <= 0 ) {
                                                                                                       username2fanli_[username].recharge_id2count_[i.second.id()] = 1;
                                                                                               } else {
                                                                                                       if (username2fanli_[username].recharge_id2count_.count(i.second.id()) <= 0) {
                                                                                                               username2fanli_.at(username).recharge_id2count_[i.second.id()] = 1;
                                                                                                       } else {
                                                                                                               username2fanli_.at(username).recharge_id2count_[i.second.id()] += 1;
                                                                                                       }
                                                                                               }
                                                                                       }
                                                                               }
                                                                       }
                                                                       done_cb_();
                                                               });
                        db_->push_message(shandang_db_msg, st);

                        auto cb_db_msg = make_shared<db::message>("load_come_back",
                                                               db::message::req_type::rt_select,
                                                               [] (const shared_ptr<db::message>& msg) {
                                                                       auto& results = msg->results();
                                                                       for (const auto& i : results) {
                                                                               ASSERT(i.size() == 1);
                                                                               come_backs_[boost::any_cast<string>(i[0])];
                                                                       }
                                                                       done_cb_();
                                                               });
                        db_->push_message(cb_db_msg, st);
                }

                void scene::process_lock_fanli_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2cr_lock_fanli_req::slfr);

                        proto::ss::base rsp_msg;
                        auto *rsp = rsp_msg.MutableExtension(ps::cr2s_lock_fanli_rsp::clfr);

                        auto result = pd::OK;
                        do {
                                auto iter = username2fanli_.find(req.username());
                                if (iter == username2fanli_.end()) {
                                        result = pd::FANLI_NO_FANLI;
                                        break;
                                }
                                auto& fanli = iter->second;
                                if (!fanli.locked_by_.empty() && fanli.locked_by_ != req.username()) {
                                        result = pd::FANLI_LOCKED_BY_OTHER;
                                        break;
                                }
                                fanli.locked_by_ = req.username();
                                auto *fanli_array = rsp->mutable_fanlis();
                                for (const auto& i : fanli.recharge_id2count_) {
                                        auto *fanlis = fanli_array->add_fanlis();
                                        fanlis->set_recharge(i.first);
                                        fanlis->set_count(i.second);
                                }
                                if (!fanli.lock_timer_) {
                                        fanli.lock_timer_ = ADD_TIMER(
                                                st_,
                                                ([this, username = req.username()] (auto canceled, const auto& timer) {

                                                        auto iter = username2fanli_.find(username);
                                                        if (iter == username2fanli_.end()) {
                                                                return;
                                                        }
                                                        auto& fanli = iter->second;
                                                        if (fanli.lock_timer_ == timer) {
                                                                fanli.lock_timer_.reset();
                                                        }
                                                        if (!canceled) {
                                                                fanli.locked_by_.clear();
                                                        }
                                                }),
                                                5s);
                                }
                        } while (false);

                        rsp->set_username(req.username());
                        rsp->set_result(result);
                        scene_mgr::instance().send_msg(id(), rsp_msg);
                }

                void scene::process_remove_fanli(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2cr_remove_fanli::srf);

                        auto iter = username2fanli_.find(req.username());
                        ASSERT(iter != username2fanli_.end());

                        auto& fanli = iter->second;
                        if (fanli.locked_by_ != req.username()) {
                                return;
                        }
                        ASSERT(fanli.lock_timer_);

                        fanli.lock_timer_->cancel();
                        fanli.lock_timer_.reset();

                        username2fanli_.erase(req.username());

                        ASSERT(db_);
                        auto db_msg = make_shared<db::message>("delete_shandang_fanli", db::message::req_type::rt_update);
                        db_msg->push_param(req.username());
                        db_->push_message(db_msg);
                }

                void scene::process_lock_come_back_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2cr_lock_come_back_req::slcbr);

                        proto::ss::base rsp_msg;
                        auto *rsp = rsp_msg.MutableExtension(ps::cr2s_lock_come_back_rsp::clcbr);

                        auto result = pd::OK;
                        do {
                                auto iter = come_backs_.find(req.username());
                                if (iter == come_backs_.end()) {
                                        result = pd::COME_BACK_NO_COME_BACK;
                                        break;
                                }
                                auto& back = iter->second;
                                if (!back.locked_by_.empty() && back.locked_by_ != req.username()) {
                                        result = pd::COME_BACK_LOCKED_BY_OTHER;
                                        break;
                                }
                                back.locked_by_ = req.username();
                                if (!back.lock_timer_) {
                                        back.lock_timer_ = ADD_TIMER(
                                                st_,
                                                ([this, username = req.username()] (auto canceled, const auto& timer) {

                                                        auto iter = come_backs_.find(username);
                                                        if (iter == come_backs_.end()) {
                                                                return;
                                                        }
                                                        auto& back = iter->second;
                                                        if (back.lock_timer_ == timer) {
                                                                back.lock_timer_.reset();
                                                        }
                                                        if (!canceled) {
                                                                back.locked_by_.clear();
                                                        }
                                                }),
                                                5s);
                                }
                        } while (false);

                        rsp->set_username(req.username());
                        rsp->set_result(result);
                        scene_mgr::instance().send_msg(id(), rsp_msg);
                }

                void scene::process_remove_come_back(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2cr_remove_come_back::srcb);

                        auto iter = come_backs_.find(req.username());
                        ASSERT(iter != come_backs_.end());

                        auto& back = iter->second;
                        if (back.locked_by_ != req.username()) {
                                return;
                        }
                        ASSERT(back.lock_timer_);

                        back.lock_timer_->cancel();
                        back.lock_timer_.reset();

                        come_backs_.erase(req.username());

                        ASSERT(db_);
                        auto db_msg = make_shared<db::message>("delete_come_back", db::message::req_type::rt_update);
                        db_msg->push_param(req.username());
                        db_->push_message(db_msg);
                }

        }
}
