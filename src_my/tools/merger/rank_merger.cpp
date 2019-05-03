#include "rank_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "utils/proto_utils.hpp"
#include "utils/game_def.hpp"
#include "config/guanpin_ptts.hpp"
#include "config/utils.hpp"
#include <list>
#include <limits>

namespace nora {
        namespace {

                static map<pd::rank_type, string> rank_type2name{
                        { pd::RT_LEVEL, "level" },
                        { pd::RT_ZHANLI, "zhanli" },
                        { pd::RT_RECEIVE_GIFT, "receive_gift" },
                        { pd::RT_GIVE_GIFT, "give_gift" },
                        { pd::RT_MANSION_FANCY, "mansion_fancy" },
                        { pd::RT_TOWER, "tower" },
                        { pd::RT_CHAOTANG, "chaotang" },
                        { pd::RT_MARRIAGE_DEVOTION, "marriage_devotion" },
                                };

        }

        void rank_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge rank begin";
                                auto type = static_cast<pd::rank_type>(rank_gid_);
                                load_rank(type);
                        });
        }

        void rank_merger::load_rank(pd::rank_type type) {
                ASSERT(st_->check_in_thread());
                if (from_rank_) {
                        from_rank_.reset();
                }
                if (to_rank_) {
                        to_rank_.reset();
                }
                auto from_db_msg = make_shared<db::message>("get_rank",
                                                            db::message::req_type::rt_select,
                                                            [this, type] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);
                                                                    from_rank_ = make_unique<pd::rank>();

                                                                    pd::rank data;
                                                                    data.ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    ILOG << " get from rank " << pd::rank_type_Name(data.type());
                                                                    *from_rank_ = data;

                                                                    if (to_rank_) {
                                                                            this->merge(type);
                                                                    }
                                                            });
                from_db_msg->push_param(rank_gid_);
                from_db_->push_message(from_db_msg, st_);
                auto to_db_msg = make_shared<db::message>("get_rank",
                                                          db::message::req_type::rt_select,
                                                          [this, type] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);
                                                                  to_rank_ = make_unique<pd::rank>();

                                                                  pd::rank data;
                                                                  data.ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  ILOG << " get to rank " << pd::rank_type_Name(data.type());
                                                                  *to_rank_ = data;

                                                                  if (from_rank_) {
                                                                          this->merge(type);
                                                                  }
                                                          });
                to_db_msg->push_param(rank_gid_);
                to_db_->push_message(to_db_msg, st_);
        }

        void rank_merger::merge(pd::rank_type type) {
                ASSERT(st_->check_in_thread());
                switch (type) {
                case pd::RT_LEVEL:
                case pd::RT_ZHANLI:
                case pd::RT_RECEIVE_GIFT:
                case pd::RT_GIVE_GIFT:
                case pd::RT_MANSION_FANCY:
                case pd::RT_TOWER:
                case pd::RT_MARRIAGE_DEVOTION:
                        normal_merge(type);
                        break;
                case pd::RT_CHAOTANG:
                        chaotang_merge(type);
                        break;
                
                default:
                        ASSERT(false);
                }

                auto db_msg = make_shared<db::message>("update_rank",
                                                       db::message::req_type::rt_update,
                                                       [this, type] (const auto& msg) {
                                                               rank_gid_++;
                                                               auto next_type = static_cast<pd::rank_type>(rank_gid_);
                                                               if (next_type == pd::RT_COUNT) {
                                                                       ILOG << "rank merge done";
                                                                       if (done_cb_) {
                                                                               done_cb_();
                                                                       }
                                                               } else {
                                                                       this->load_rank(next_type);
                                                               }
                                                       });
                string data_str;
                to_rank_->SerializeToString(&data_str);
                db_msg->push_param(rank_gid_);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);
        }

        void rank_merger::chaotang_merge(pd::rank_type type) {
                ASSERT(st_->check_in_thread());
                map<int32_t, vector<pd::rank_entity>> gpin2guans;

                for (const auto& i : to_rank_->entities().entities()) {
                        if (i.values(0) == GUANPIN_HIGHEST_GPIN) {
                                gpin2guans[GUANPIN_HIGHEST_GPIN - 1].push_back(i);
                        }
                }

                for (const auto& i : from_rank_->entities().entities()) {
                        if (i.values(0) == GUANPIN_HIGHEST_GPIN) {
                                gpin2guans[GUANPIN_HIGHEST_GPIN - 1].push_back(i);
                        }
                }

                auto merger_gpin = GUANPIN_HIGHEST_GPIN - 1u;
                PTTS_LOAD(guanpin);

                while (merger_gpin > 0) {
                        auto from_idx = 0u;
                        auto to_idx = 0u;
                        auto cur_gpin = merger_gpin;

                        vector<pd::rank_entity> from_guans;
                        vector<pd::rank_entity> to_guans;

                        for (const auto& i : to_rank_->entities().entities()) {
                                if (i.values(0) == merger_gpin) {
                                        to_guans.push_back(i);
                                }
                        }

                        for (const auto& i : from_rank_->entities().entities()) {
                                if (i.values(0) == merger_gpin) {
                                        from_guans.push_back(i);
                                }
                        }

                        bool to_turn = true;
                        while (cur_gpin > 0 && (to_idx < to_guans.size() || from_idx < from_guans.size())) {
                                const auto& gpin_ptt = PTTS_GET(guanpin, cur_gpin);
                                auto& guans = gpin2guans[cur_gpin];
                                if (gpin_ptt.has_max_guan() && (gpin_ptt.max_guan() - gpin_ptt.dianshi().max_luqu()) <= guans.size()) {
                                        cur_gpin--;
                                } else {
                                        if (to_turn) {
                                                if (to_idx < to_guans.size()) {
                                                        guans.push_back(to_guans[to_idx]);
                                                        to_idx++;
                                                }
                                                to_turn = !to_turn;
                                        } else {
                                                if (from_idx < from_guans.size()) {
                                                        guans.push_back(from_guans[from_idx]);
                                                        from_idx++;
                                                }
                                                to_turn = !to_turn;
                                        }
                                }
                        }
                        merger_gpin--;
                }

                to_rank_ = make_unique<pd::rank>();
                to_rank_->set_type(type);

                auto *e = to_rank_->mutable_entities();
                for (auto iter = gpin2guans.rbegin(); iter != gpin2guans.rend(); iter++) {
                        auto index = numeric_limits<int>::max();
                        for (const auto& j : (*iter).second) {
                                pd::rank_entity entity;
                                entity.set_gid(j.gid());
                                entity.add_values((*iter).first);
                                entity.add_values(index);
                                entity.set_like_count(j.like_count());
                                *e->add_entities() = entity;
                                index--;
                        }
                }
        }

        void rank_merger::normal_merge(pd::rank_type type) {
                ASSERT(st_->check_in_thread());
                list<pd::rank_entity> entites;

                for (const auto& i : to_rank_->entities().entities()) {
                        entites.push_back(i);
                }

                auto iter = entites.begin();
                for (const auto& i : from_rank_->entities().entities()) {
                        auto need_push = false;
                        for (auto j = iter; j != entites.end(); j++) {
                                if (i > (*j)) {
                                        need_push = true;
                                        entites.insert(j, i);
                                        break;
                                }
                        }

                        if (!need_push) {
                                entites.push_back(i);
                        }
                }

                to_rank_ = make_unique<pd::rank>();
                to_rank_->set_type(type);

                auto *e = to_rank_->mutable_entities();
                for (const auto& i : entites) {
                        *e->add_entities() = i;
                }
        }
}
