#include "prepare.hpp"
#include "utils/assert.hpp"
#include <string>

namespace pd = proto::data;

namespace nora {

        void prepare::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ++waiting_count_;
                                auto db_msg = make_shared<db::message>("load_gid_and_rolename",
                                                                       db::message::req_type::rt_select,
                                                                       [this] (const auto& msg) {
                                                                               auto& results = msg->results();
                                                                               for (const auto& i : results) {
                                                                                       ASSERT(i.size() == 2);
                                                                                       auto gid = boost::any_cast<uint64_t>(i[0]);
                                                                                       auto rolename = boost::any_cast<string>(i[1]);
                                                                                       ASSERT(from_gid2rolename_.count(gid) == 0);
                                                                                       ASSERT(from_rolename2gid_.count(rolename) == 0);
                                                                                       from_gid2rolename_[gid] = rolename;
                                                                                       from_rolename2gid_[rolename] = gid;
                                                                               }
                                                                               if (--waiting_count_ == 0 && done_cb_) {
                                                                                       done_cb_();
                                                                               }
                                                                       });
                                from_db_->push_message(db_msg, st_);

                                ++waiting_count_;
                                db_msg = make_shared<db::message>("load_gid_and_rolename",
                                                                  db::message::req_type::rt_select,
                                                                  [this] (const auto& msg) {
                                                                          auto& results = msg->results();
                                                                          for (const auto& i : results) {
                                                                                  ASSERT(i.size() == 2);
                                                                                  auto gid = boost::any_cast<uint64_t>(i[0]);
                                                                                  auto rolename = boost::any_cast<string>(i[1]);
                                                                                  ASSERT(to_gid2rolename_.count(gid) == 0);
                                                                                  ASSERT(to_rolename2gid_.count(rolename) == 0);
                                                                                  to_gid2rolename_[gid] = rolename;
                                                                                  to_rolename2gid_[rolename] = gid;
                                                                          }
                                                                          if (--waiting_count_ == 0 && done_cb_) {
                                                                                  done_cb_();
                                                                          }
                                                                  });
                                to_db_->push_message(db_msg, st_);

                                ++waiting_count_;
                                db_msg = make_shared<db::message>("get_league_separations",
                                                                db::message::req_type::rt_select,
                                                                [this] (const auto& msg) {
                                                                        auto& results = msg->results();
                                                                        for (const auto& i : results) {
                                                                                ASSERT(i.size() == 2);
                                                                                auto gid = boost::any_cast<uint64_t>(i[0]);
                                                                                pd::league_separation separation;
                                                                                separation.ParseFromString(boost::any_cast<string>(i[1]));
                                                                                auto league_name = separation.name();
                                                                                ASSERT(from_gid2league_separation_.count(gid) == 0);
                                                                                ASSERT(from_league_name2gid_.count(league_name) == 0);
                                                                                from_gid2league_separation_[gid] = separation;
                                                                                from_league_name2gid_[league_name] = gid;
                                                                        }
                                                                        if (--waiting_count_ == 0 && done_cb_) {
                                                                                done_cb_();
                                                                        }
                                                               });
                                from_db_->push_message(db_msg, st_);

                                ++waiting_count_;
                                db_msg = make_shared<db::message>("get_league_separations",
                                                                db::message::req_type::rt_select,
                                                                [this] (const auto& msg) {
                                                                        auto& results = msg->results();
                                                                        for (const auto& i : results) {
                                                                                ASSERT(i.size() == 2);
                                                                                auto gid = boost::any_cast<uint64_t>(i[0]);
                                                                                pd::league_separation separation;
                                                                                separation.ParseFromString(boost::any_cast<string>(i[1]));
                                                                                auto league_name = separation.name();
                                                                                ASSERT(to_gid2league_separation_.count(gid) == 0);
                                                                                ASSERT(to_league_name2gid_.count(league_name) == 0);
                                                                                to_gid2league_separation_[gid] = separation;
                                                                                to_league_name2gid_[league_name] = gid;
                                                                        }
                                                                        if (--waiting_count_ == 0 && done_cb_) {
                                                                                done_cb_();
                                                                        }
                                                                });
                                to_db_->push_message(db_msg, st_);
                        });
        }

}
