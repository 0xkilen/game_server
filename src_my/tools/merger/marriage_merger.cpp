#include "marriage_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"

namespace nora {

        void marriage_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge marriage begin";
                                load();
                        });
        }

        void marriage_merger::load() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("get_marriage",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);

                                                                    from_marriage_ = make_unique<pd::marriage_data>();
                                                                    from_marriage_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    if (to_marriage_) {
                                                                            this->merge();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);
                auto to_db_msg = make_shared<db::message>("get_marriage",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);

                                                                  to_marriage_ = make_unique<pd::marriage_data>();
                                                                  to_marriage_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  if (from_marriage_) {
                                                                          this->merge();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }


        void marriage_merger::merge() {
                ASSERT(st_->check_in_thread());
                for (const auto& i : from_marriage_->star_wishes().star_wishes()) {
                        *to_marriage_->mutable_star_wishes()->add_star_wishes() = i;
                }
                for (const auto& i : from_marriage_->pks().pks()) {
                        *to_marriage_->mutable_pks()->add_pks() = i;
                }
                for (const auto& i : from_marriage_->qiu_qians().actors()) {
                        *to_marriage_->mutable_qiu_qians()->add_actors() = i;
                }
                for (const auto& i : from_marriage_->qiu_qians().pregnencies()) {
                        *to_marriage_->mutable_qiu_qians()->add_pregnencies() = i;
                }

                auto db_msg = make_shared<db::message>("save_marriage",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               if (done_cb_) {
                                                                       done_cb_();
                                                               }
                                                       });
                string data_str;
                to_marriage_->SerializeToString(&data_str);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);
        }

}
