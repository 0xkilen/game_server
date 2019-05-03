#include "huanzhuang_pvp_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"

namespace nora {

        void huanzhuang_pvp_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge huanzhuang_pvp begin";
                                load();
                        });
        }

        void huanzhuang_pvp_merger::load() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("get_huanzhuang_pvp",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);

                                                                    from_huanzhuang_pvp_ = make_unique<pd::huanzhuang_pvp>();
                                                                    from_huanzhuang_pvp_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    if (to_huanzhuang_pvp_) {
                                                                            this->merge();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);
                auto to_db_msg = make_shared<db::message>("get_huanzhuang_pvp",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);

                                                                  to_huanzhuang_pvp_ = make_unique<pd::huanzhuang_pvp>();
                                                                  to_huanzhuang_pvp_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  if (from_huanzhuang_pvp_) {
                                                                          this->merge();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }


        void huanzhuang_pvp_merger::merge() {
                ASSERT(st_->check_in_thread());

                // base data
                ASSERT(to_huanzhuang_pvp_->base_data().start_day() == from_huanzhuang_pvp_->base_data().start_day());
                ASSERT(to_huanzhuang_pvp_->base_data().pttid() == from_huanzhuang_pvp_->base_data().pttid());

                // male and female rank
                for (const auto& i : from_huanzhuang_pvp_->male_rank().roles()) {
                        *to_huanzhuang_pvp_->mutable_male_rank()->add_roles() = i;
                }
                to_huanzhuang_pvp_->mutable_male_rank()->clear_history_champions();
                to_huanzhuang_pvp_->mutable_male_rank()->set_votes(to_huanzhuang_pvp_->male_rank().votes() + from_huanzhuang_pvp_->male_rank().votes());
                for (const auto& i : from_huanzhuang_pvp_->female_rank().roles()) {
                        *to_huanzhuang_pvp_->mutable_female_rank()->add_roles() = i;
                }
                to_huanzhuang_pvp_->mutable_female_rank()->clear_history_champions();
                to_huanzhuang_pvp_->mutable_female_rank()->set_votes(to_huanzhuang_pvp_->female_rank().votes() + from_huanzhuang_pvp_->female_rank().votes());

                // active roles
                for (auto i : from_huanzhuang_pvp_->active_roles().cur()) {
                        to_huanzhuang_pvp_->mutable_active_roles()->add_cur(i);
                }
                for (auto i : from_huanzhuang_pvp_->active_roles().next()) {
                        to_huanzhuang_pvp_->mutable_active_roles()->add_next(i);
                }

                // best_ranks
                to_huanzhuang_pvp_->clear_best_ranks();

                auto db_msg = make_shared<db::message>("save_huanzhuang_pvp",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                                ILOG << "huanzhuang_pvp merge done";
                                                               if (done_cb_) {
                                                                       done_cb_();
                                                               }
                                                       });
                string data_str;
                to_huanzhuang_pvp_->SerializeToString(&data_str);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);
        }

}
