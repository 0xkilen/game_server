#include "fund.hpp"
#include "log.hpp"
#include "proto/data_activity.pb.h"
#include "config/options_ptts.hpp"
#include "utils/proto_utils.hpp"
#include <limits>

namespace nora {
        namespace scene {

                fund::fund(const shared_ptr<service_thread>& st) {
                        st_ = st;
                        auto ptt = PTTS_GET_COPY(options, 1u);
                        gamedb_ = make_shared<db::connector>(ptt.game_db().ipport(),
                                                             ptt.game_db().user(),
                                                             ptt.game_db().password(),
                                                             ptt.game_db().database());
                        gamedb_->start();
                }

                void fund::start() {
                        auto db_msg = make_shared<db::message>("get_activity_mgr",
                                                               db::message::req_type::rt_select,
                                                               [this, self = this->shared_from_this()] (const shared_ptr<db::message>& msg) {
                                                                       auto& results = msg->results();
                                                                       if (!results.empty()) {
                                                                               ASSERT(results.size() == 1);
                                                                               const auto& result = results.front();
                                                                               ASSERT(result.size() == 1);
                                                                               pd::activity_mgr activity_mgr;
                                                                               activity_mgr.ParseFromString(boost::any_cast<string>(result[0]));
                                                                               if (activity_mgr.has_fund()) {
                                                                                       fund_ = activity_mgr.fund().value();
                                                                               }
                                                                       }
                                                               });
                        gamedb_->push_message(db_msg, st_);
                }

                void fund::update_activity_fund() {
                        auto db_msg = make_shared<db::message>("save_activity_mgr", db::message::req_type::rt_update);
                        pd::activity_mgr am;
                        am.mutable_fund()->set_value(fund_);
                        string amstr;
                        am.SerializeToString(&amstr);
                        db_msg->push_param(amstr);
                        gamedb_->push_message(db_msg, st_);
                }

                void fund::activity_add_fund(bool additional_count) {
                        {
                                lock_guard<mutex> lock(lock_);
                                fund_ += 1;
                        }
                        update_activity_fund();
                        if (additional_count) {
                                timer_ = ADD_TIMER(
                                        st_,
                                        ([this] (auto canceled, const auto& timer) {
                                                if (timer_ == timer) {
                                                        timer_.reset();
                                                }
                                                if (!canceled) {
                                                        {
                                                                lock_guard<mutex> lock(lock_);
                                                                fund_ += rand() % 5 + 1;
                                                        }
                                                        this->update_activity_fund();
                                                }
                                        }),
                                        2s);
                        }
                }

                void fund::gm_activity_add_fund(uint32_t count) {
                        {
                                lock_guard<mutex> lock(lock_);
                                fund_ += count;
                        }
                        update_activity_fund();
                }

                void fund::stop() {
                        if (gamedb_) {
                                gamedb_->stop();
                        }
                        if (timer_) {
                                timer_->cancel();
                                timer_.reset();
                        }
                }

        }
}
