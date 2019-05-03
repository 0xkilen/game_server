#include "league_war_merger.hpp"
#include "tools/log.hpp"
#include "utils/assert.hpp"
#include "prepare.hpp"
#include "utils/gid.hpp"

namespace nora {

        string fix_collide_name(uint64_t gid, const string& name) {
                string ret = name;
                ret += 's';
                ret += to_string(gid::instance().get_server_id(gid));
                return ret;
        }

        void league_war_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge league_war begin";
                                load();
                        });
        }

        void league_war_merger::load() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("get_league_war",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);

                                                                    from_league_war_ = make_unique<pd::league_war>();
                                                                    from_league_war_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    if (to_league_war_) {
                                                                            this->merge();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);

                auto to_db_msg = make_shared<db::message>("get_league_war",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);

                                                                  to_league_war_ = make_unique<pd::league_war>();
                                                                  to_league_war_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  if (from_league_war_) {
                                                                          this->merge();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }

        void league_war_merger::merge() {
                ASSERT(st_->check_in_thread());

                for (auto& i : from_league_war_->enrolled_leagues().enrolled_infos()) {
                        *to_league_war_->mutable_enrolled_leagues()->add_enrolled_infos() = i;
                }

                to_league_war_->clear_league_war_info();
                to_league_war_->clear_league_war_reward();

                auto db_msg = make_shared<db::message>("save_league_war",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               TOOL_TLOG << "merge league_war done";
                                                               if (done_cb_) {
                                                                       done_cb_();
                                                               }
                                                       });
                string data_str;
                to_league_war_->SerializeToString(&data_str);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);
        }

}
