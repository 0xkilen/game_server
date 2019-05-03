#include "guanpin_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "utils/game_def.hpp"
#include "config/guanpin_ptts.hpp"
#include "config/mail_ptts.hpp"
#include "config/utils.hpp"
#include "utils/gid.hpp"
#include <map>
#include <vector>

namespace nora {

        void guanpin_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge guanpin begin";
                                PTTS_LOAD(guanpin);
                                PTTS_LOAD(mail);
                                load_rank();
                        });
        }

        void guanpin_merger::load_rank() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("get_guanpin",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);

                                                                    from_guanpin_ = make_unique<pd::guanpin>();
                                                                    from_guanpin_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    if (to_guanpin_) {
                                                                            this->merge_rank();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);
                auto to_db_msg = make_shared<db::message>("get_guanpin",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);

                                                                  to_guanpin_ = make_unique<pd::guanpin>();
                                                                  to_guanpin_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  if (from_guanpin_) {
                                                                          this->merge_rank();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }

        void guanpin_merger::merge_rank() {
                ASSERT(st_->check_in_thread());
                map<int32_t, vector<uint64_t>> gpin2guans;
                
                for (const auto& i : to_guanpin_->ranks().ranks()) {
                        if (i.gpin() == GUANPIN_HIGHEST_GPIN && i.has_guans()) {
                                ASSERT(i.guans().gids_size() == 1);
                                gpin2guans[GUANPIN_HIGHEST_GPIN - 1].push_back(i.guans().gids(0));
                                const auto& ptt = PTTS_GET(guanpin, GUANPIN_HIGHEST_GPIN);
                                send_mail(i.guans().gids(0), ptt.merge_demote_mail(), pd::dynamic_data());
                        }
                }

                for (const auto& i : from_guanpin_->ranks().ranks()) {
                        if (i.gpin() == GUANPIN_HIGHEST_GPIN && i.has_guans()) {
                                ASSERT(i.guans().gids_size() == 1);
                                gpin2guans[GUANPIN_HIGHEST_GPIN - 1].push_back(i.guans().gids(0));
                                const auto& ptt = PTTS_GET(guanpin, GUANPIN_HIGHEST_GPIN);
                                send_mail(i.guans().gids(0), ptt.merge_demote_mail(), pd::dynamic_data());
                        }
                }
                auto merger_gpin = GUANPIN_HIGHEST_GPIN - 1;
                
                while (merger_gpin > 0) {
                        ILOG << "merge guanpin " << merger_gpin;
                        auto from_idx = 0;
                        auto to_idx = 0;
                        auto cur_gpin = merger_gpin;

                        pd::gid_array from_guans;
                        pd::gid_array to_guans;

                        for (const auto& i : to_guanpin_->ranks().ranks()) {
                                if (i.gpin() == merger_gpin && i.has_guans()) {
                                        to_guans = i.guans();
                                        break;
                                }
                        }

                        for (const auto& i : from_guanpin_->ranks().ranks()) {
                                if (i.gpin() == merger_gpin && i.has_guans()) {
                                        from_guans = i.guans();
                                        break;
                                }
                        }
                        
                        bool to_turn = true;
                        while (cur_gpin > 0 && (to_idx < to_guans.gids_size() || from_idx < from_guans.gids_size())) {
                                const auto& gpin_ptt = PTTS_GET(guanpin, cur_gpin);
                                auto& guans = gpin2guans[cur_gpin];
                                if (gpin_ptt.has_max_guan() && (gpin_ptt.max_guan() - gpin_ptt.dianshi().max_luqu()) <= guans.size()) {
                                        cur_gpin--;
                                } else {
                                        if (to_turn) {
                                                if (to_idx < to_guans.gids_size()) {
                                                        if (cur_gpin != merger_gpin) {
                                                                const auto& target_gpin_ptt = PTTS_GET(guanpin, merger_gpin);
                                                                send_mail(to_guans.gids(to_idx), target_gpin_ptt.merge_demote_mail(), pd::dynamic_data());
                                                        }
                                                        guans.push_back(to_guans.gids(to_idx));
                                                        to_idx++;
                                                }
                                                to_turn = !to_turn;
                                        } else {
                                                if (from_idx < from_guans.gids_size()) {
                                                        if (cur_gpin != merger_gpin) {
                                                                const auto& target_gpin_ptt = PTTS_GET(guanpin, merger_gpin);
                                                                send_mail(from_guans.gids(from_idx), target_gpin_ptt.merge_demote_mail(), pd::dynamic_data());
                                                        }
                                                        guans.push_back(from_guans.gids(from_idx));
                                                        from_idx++;
                                                }
                                                to_turn = !to_turn;
                                        }
                                }
                        }
                        merger_gpin--;
                }

                to_guanpin_ = make_unique<pd::guanpin>();
                auto *r = to_guanpin_->mutable_ranks();
                for (const auto& i : gpin2guans) {
                        auto *add_ranks = r->add_ranks();
                        add_ranks->set_gpin(i.first);
                        auto *g = add_ranks->mutable_guans();
                        for (auto j : i.second) {
                                g->add_gids(j);
                        }
                }

                auto db_msg = make_shared<db::message>("save_guanpin",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               this->load_next_page();
                                                       });
                string data_str;
                to_guanpin_->SerializeToString(&data_str);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);
        }

        void guanpin_merger::load_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_guans",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_guans_done(msg);
                                                       });
                db_msg->push_param(cur_page_ * page_size_);
                db_msg->push_param(page_size_);
                from_db_->push_message(db_msg, st_);
                cur_page_ += 1;
        }

        void guanpin_merger::on_db_get_guans_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        ILOG << "merge guanpin done";
                        if (done_cb_) {
                                done_cb_();
                        }
                        return;
                }

                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 2);
                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("add_guan",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });
                        auto gid = boost::any_cast<uint64_t>(i[0]);
                        ILOG << "process guan " << gid;
                        db_msg->push_param(gid);
                        db_msg->push_param(boost::any_cast<string>(i[1]));
                        to_db_->push_message(db_msg, st_);
                }
        }

        void guanpin_merger::send_mail(uint64_t role, uint32_t mail_pttid, const pd::dynamic_data& dynamic_data) {
                auto m = mail_new_mail(role, mail_pttid, dynamic_data);
                auto db_msg = make_shared<db::message>("add_mail", db::message::req_type::rt_insert);
                db_msg->push_param(m.gid());
                db_msg->push_param(role);
                string mail_str;
                m.SerializeToString(&mail_str);
                db_msg->push_param(mail_str);
                to_db_->push_message(db_msg);
        }

        pd::mail guanpin_merger::mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data) {
                const auto& ptt = PTTS_GET(mail, pttid);
                auto events = ptt.events();
                pd::mail m;
                m.set_gid(gid::instance().new_gid(gid_type::MAIL));
                if (dynamic_data.has_time()) {
                        m.set_time(dynamic_data.time());
                } else {
                        m.set_time(system_clock::to_time_t(system_clock::now()));
                }
                m.set_pttid(pttid);
                *m.mutable_dynamic_data() = dynamic_data;
                *m.mutable_events() = events;
                return m;
        }
}