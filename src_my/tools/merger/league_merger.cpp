#include "league_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "utils/gid.hpp"
#include "proto/data_league.pb.h"
#include "config/utils.hpp"
#include "config/mail_ptts.hpp"
#include "config/league_ptts.hpp"
#include <chrono>

namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {
        namespace {
        	    string fix_collide_name(uint64_t gid, const string& name) {
                        string ret = name;
                        ret += 's';
                        ret += to_string(gid::instance().get_server_id(gid));
                        return ret;
                }

                int compare_entity(const pd::rank_entity& a, const pd::rank_entity& b) {
                        auto a_level = a.values(0);
                        auto b_level = b.values(0);
                        if (a_level != b_level) {
                                return a_level - b_level;
                        }
                        auto a_exp = a.values(1);
                        auto b_exp = b.values(1);
                        if (a_exp != b_exp) {
                                return a_exp - b_exp;
                        }

                        auto a_exp_update_time = a.values(2);
                        auto b_exp_update_time = b.values(2);
                        return b_exp_update_time - a_exp_update_time;
                }

                int compare_city_league(const pd::city_league_entity& a, const pd::city_league_entity& b) {
                        ASSERT(a.values_size() > 0);
                        ASSERT(b.values_size() > 0);
                        if (a.values(0) != b.values(0)) {
                                return a.values(0) - b.values(0);
                        }
                        return b.values(1) - b.values(1);
                }

                void reset_city_management(pd::city& city) {
                        const auto& city_ptt = PTTS_GET(city, static_cast<uint32_t>(city.gid()));
                        if (city.management_league().has_gid()) {
                                city.mutable_management_league()->clear_gid();
                                city.mutable_management_league()->set_system_pttid(city_ptt.origin_league());
                                city.mutable_management_league()->set_protect_end_time(0);
                                city.mutable_management_league()->set_defence_times(0);
                                city.mutable_management_league()->set_urban_level(0);
                        }
                }
        }
	void league_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge league begin";
                                PTTS_LOAD(city);
                                PTTS_LOAD(league);
                                PTTS_LOAD(mail);
                                load_next_page();
                        });
        }

       	void league_merger::load_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_leagues",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_leagues_done(msg);
                                                       });
                db_msg->push_param(cur_page_ * page_size_);
                db_msg->push_param(page_size_);
                from_db_->push_message(db_msg, st_);
                cur_page_ += 1;
	}

	void league_merger::on_db_get_leagues_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        ILOG << "league from merge to to done";
                        fix_to_names();
                        return;
                }

		const auto& to_league_name2gid = prepare::instance().to_league_name2gid();
                const auto& to_gid2league_separation = prepare::instance().to_gid2league_separation();
		const auto& from_gid2league_separation = prepare::instance().from_gid2league_separation();
                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 3);
                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("add_league",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });
                        auto gid = boost::any_cast<uint64_t>(i[0]);
                        db_msg->push_param(gid);

                        ASSERT(from_gid2league_separation.count(gid) > 0);
                        auto separation = from_gid2league_separation.at(gid);
                        auto league_name = separation.name();
                        ILOG << "process league " << gid << " " << league_name;

                        bool name_fixed = false;
                        for (const auto& j : to_league_name2gid) {
                                ILOG << j.first << " " << j.second;
                        }

                        if (to_league_name2gid.count(league_name) > 0) {
                                auto to_gid = to_league_name2gid.at(league_name);
                                ILOG << league_name << " already exist, from: " << gid << " to: " << to_gid;
                                ASSERT(to_gid2league_separation.count(to_gid) > 0);
                                const auto& to_separation = to_gid2league_separation.at(to_gid);
                                if (compare_entity(separation.rank_entity(), to_separation.rank_entity()) <= 0) {
                                        auto new_name = fix_collide_name(gid, league_name);
                                        separation.set_name(new_name);
                                        pd::league league_data;
                                        league_data.ParseFromString(boost::any_cast<string>(i[1]));
                                        league_data.mutable_info()->set_name(new_name);
                                        string league_data_str;
                                        league_data.SerializeToString(&league_data_str);
                                        db_msg->push_param(league_data_str);
                                        ILOG << "fix to league name with server suffix";
                                        name_fixed = true;

                                        const auto& ptt = PTTS_GET(league, 1);
                                        pd::dynamic_data dd;
                                        dd.add_name(league_name);
                                        for (const auto& position_members : league_data.members().position_members()) {
                                            if (position_members.position() == pd::LMP_PRESIDENT) {
                                                send_mail(position_members.roles(0).gid(), ptt.merge_compensation_mail(), dd);
                                                break;
                                            }
                                        }
                                }
                                else {
                                	fix_name_to_leagues_.insert(to_gid);
                                }
                        }
                        if (!name_fixed) {
                                db_msg->push_param(boost::any_cast<string>(i[1]));
                        }

                        to_db_->push_message(db_msg, st_);

                        adding_count_ += 1;
                        db_msg = make_shared<db::message>("add_league_separation",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });

                        string from_separation_str;
	               	separation.SerializeToString(&from_separation_str);
                        db_msg->push_param(from_separation_str);
                        to_db_->push_message(db_msg, st_);
                }

	}

	void league_merger::fix_to_names() {
                ASSERT(st_->check_in_thread());
                if (fix_name_to_leagues_.empty()) {
                        this->get_cities();
                        // if (done_cb_) {
                        //         done_cb_();
                        // }
                        return;
                }

                for (auto i : fix_name_to_leagues_) {
                        auto db_msg = make_shared<db::message>("get_league",
                                                               	db::message::req_type::rt_select,
                                                               	[this] (const auto& msg) {
                                                               		this->on_db_get_fix_name_to_league_done(msg);
                                                               	});
                        db_msg->push_param(i);
                        to_db_->push_message(db_msg, st_);
                }
        }

        void league_merger::on_db_get_fix_name_to_league_done(const shared_ptr<db::message>& msg) {
        	ASSERT(st_->check_in_thread());
                ASSERT(msg->results().size() == 1);
                const auto& result = msg->results()[0];
                ASSERT(result.size() == 2);
                auto gid = boost::any_cast<uint64_t>(result[0]);
                ILOG << "fix from league " << gid << " name with server suffix";
                auto db_msg = make_shared<db::message>("update_league",
                                                       	db::message::req_type::rt_update,
                                                       	[this, gid] (const auto& msg) {
                                                               	fix_name_to_leagues_.erase(gid);
                                                               	if (fix_name_to_leagues_.empty()) {
                                                                       	this->get_cities();
                                                               	}
                                                       	});
                pd::league league_data;
                league_data.ParseFromString(boost::any_cast<string>(result[1]));
                const auto& to_gid2league_separation = prepare::instance().to_gid2league_separation();
                ASSERT(to_gid2league_separation.count(gid) > 0);
                auto league_separation = to_gid2league_separation.at(gid);

	            auto new_name = fix_collide_name(gid, league_separation.name());
                league_data.mutable_info()->set_name(new_name);
                league_separation.set_name(new_name);

               	string league_data_str;
	            league_data.SerializeToString(&league_data_str);

                string league_separation_str;
               	league_separation.SerializeToString(&league_separation_str);
                db_msg->push_param(gid);
                db_msg->push_param(league_data_str);
                db_msg->push_param(league_separation_str);
                to_db_->push_message(db_msg, st_);

                const auto& ptt = PTTS_GET(league, 1);
                pd::dynamic_data dd;
                dd.add_name(league_separation.name());
                for (const auto& position_members : league_data.members().position_members()) {
                    if (position_members.position() == pd::LMP_PRESIDENT) {
                        send_mail(position_members.roles(0).gid(), ptt.merge_compensation_mail(), dd);
                        break;
                    }
                }
        }

        void league_merger::get_cities() {
                ++waiting_get_cities_;
                auto db_msg = make_shared<db::message>("get_cities",
                                                        db::message::req_type::rt_select,
                                                        [this] (const auto& msg) {
                                                                for (const auto& i : msg->results()) {
                                                                        ASSERT(i.size() == 2);
                                                                        uint64_t city_gid = boost::any_cast<uint64_t>(i[0]);
                                                                        to_cities_[city_gid].ParseFromString(boost::any_cast<string>(i[1]));
                                                                }
                                                                if (--waiting_get_cities_ == 0) {
                                                                        this->merge_cities();
                                                                }
                                                        });
                to_db_->push_message(db_msg, st_);

                ++waiting_get_cities_;
                db_msg = make_shared<db::message>("get_cities",
                                                        db::message::req_type::rt_select,
                                                        [this] (const auto& msg) {
                                                                for (const auto& i : msg->results()) {
                                                                        ASSERT(i.size() == 2);
                                                                        uint64_t city_gid = boost::any_cast<uint64_t>(i[0]);
                                                                        from_cities_[city_gid].ParseFromString(boost::any_cast<string>(i[1]));
                                                                }
                                                                if (--waiting_get_cities_ == 0) {
                                                                        this->merge_cities();
                                                                }
                                                        });
                from_db_->push_message(db_msg, st_);
        }

        void league_merger::merge_cities() {
                waiting_update_city_ = 0;
                const auto& from_gid2league_separation = prepare::instance().from_gid2league_separation();
                const auto& to_gid2league_separation = prepare::instance().to_gid2league_separation();

                for (const auto& i : to_cities_) {
                        auto& city_data = i.second;
                        if (city_data.management_league().has_gid()) {
                                const auto& separation = to_gid2league_separation.at(city_data.management_league().gid());
                                const auto& ptt = PTTS_GET(city, static_cast<uint32_t>(city_data.gid()));
                                for (auto j : separation.members().gids()) {
                                        pd::dynamic_data dd;
                                        dd.set_city_pttid(ptt.pttid());
                                        send_mail(j, ptt.merge_compensation_mail(), dd);
                                }
                        }
                }

                for (const auto& i : from_cities_) {
                        auto& city_data = i.second;
                        if (city_data.management_league().has_gid()) {
                                const auto& separation = from_gid2league_separation.at(city_data.management_league().gid());
                                const auto& ptt = PTTS_GET(city, static_cast<uint32_t>(city_data.gid()));
                                for (auto j : separation.members().gids()) {
                                        pd::dynamic_data dd;
                                        dd.set_city_pttid(ptt.pttid());
                                        send_mail(j, ptt.merge_compensation_mail(), dd);
                                }
                        }
                }

                for (auto& i : to_cities_) {
                        ++waiting_update_city_;
                        auto city_gid = i.first;
                        auto& to_city_data = i.second;

                        reset_city_management(to_city_data);

                        list<pd::city_league_entity> entity_list;

                        for (const auto& j : to_city_data.ranking().ranking()) {
                                entity_list.push_back(j);
                        }

                        if (from_cities_.count(city_gid) > 0) {
                                auto& from_city_data = from_cities_.at(city_gid);
                                for (const auto& j : from_city_data.ranking().ranking()) {
                                        if (j.has_gid()) {
                                                entity_list.push_back(j);
                                        }
                                }

                                for (const auto& j : from_city_data.city_leagues().leagues()) {
                                        *to_city_data.mutable_city_leagues()->add_leagues() = j;
                                }
                        }
                        else {
                                auto cur_time = system_clock::to_time_t(system_clock::now());
                                for (const auto& j : from_gid2league_separation) {
                                        pd::city_league cl;
                                        auto gid = j.first;
                                        cl.set_gid(gid);
                                        cl.set_friendliness(0);
                                        cl.set_friendliness_update_time(cur_time);
                                        
                                        *to_city_data.mutable_city_leagues()->add_leagues() = cl;

                                        pd::city_league_entity new_entity;
                                        new_entity.set_gid(gid);
                                        new_entity.add_values(0);
                                        new_entity.add_values(cur_time);
                                        entity_list.push_back(new_entity);
                                }

                        }

                        entity_list.sort(compare_city_league);

                        to_city_data.mutable_ranking()->clear_ranking();
                        for (const auto& j : entity_list) {
                                *to_city_data.mutable_ranking()->add_ranking() = j;
                        }

                        auto db_msg = make_shared<db::message>("update_city",
                                        db::message::req_type::rt_update,
                                        [this] (const auto& msg) {
                                                if (--waiting_update_city_ == 0) {
                                                        ILOG << "league merge done";
                                                        if (done_cb_) {
                                                                done_cb_();
                                                        }
                                                }
                                        });
                        string city_str;
                        to_city_data.SerializeToString(&city_str);
                        db_msg->push_param(city_gid);
                        db_msg->push_param(city_str);                        
                        to_db_->push_message(db_msg, st_);
                }
        }

        void league_merger::send_mail(uint64_t role, uint32_t mail_id, const pd::dynamic_data& dynamic_data) {
                ILOG << "send role change names mail to role: " << role;
                auto m = mail_new_mail(role, mail_id, dynamic_data);
                auto db_msg = make_shared<db::message>("add_mail", db::message::req_type::rt_insert);
                db_msg->push_param(m.gid());
                db_msg->push_param(role);
                string mail_str;
                m.SerializeToString(&mail_str);
                db_msg->push_param(mail_str);
                to_db_->push_message(db_msg);
        }

        pd::mail league_merger::mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data) {
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