#include "conflict_battle_records.hpp"
#include "config/options_ptts.hpp"
#include <limits>

namespace nora {
        namespace scene {



                conflict_battle_records::conflict_battle_records() {
                        auto ptt = PTTS_GET_COPY(options, 1u);
                        gamedb_ = make_shared<db::connector>(ptt.game_db().ipport(),
                                                             ptt.game_db().user(),
                                                             ptt.game_db().password(),
                                                             ptt.game_db().database());
                        gamedb_->start();
                }

                void conflict_battle_records::add_record(const pd::conflict_battle_record& record) {
                        lock_guard<mutex> lock(lock_);
                        auto db_msg = make_shared<db::message>("add_conflict_battle_record", db::message::req_type::rt_insert);
                        string str;
                        record.SerializeToString(&str);
                        db_msg->push_param(str);
                        gamedb_->push_message(db_msg);
                }

                void conflict_battle_records::record(uint64_t gid, const function<void(pd::result, uint64_t, const pd::conflict_battle_record&)>& cb, const shared_ptr<service_thread>& st) const{
                        auto db_msg = make_shared<db::message>("get_conflict_battle_records", db::message::req_type::rt_select,
                                                                [this, gid, cb] (const shared_ptr<db::message>& msg){
                                                                        auto& results = msg->results();
                                                                        if (!results.empty()) {
                                                                                ASSERT(results.size() == 1);
                                                                                const auto& result = results.front();
                                                                                ASSERT(result.size() == 1);
                                                                                pd::conflict_battle_record record;
                                                                                record.ParseFromString(boost::any_cast<string>(result[0]));
                                                                                cb(pd::OK, gid, record);
                                                                        } else {
                                                                                cb(pd::NOT_FOUND, gid, pd::conflict_battle_record());
                                                                        }

                                                                });
                        db_msg->push_param(gid);
                        gamedb_->push_message(db_msg, st);
                }
        }
}