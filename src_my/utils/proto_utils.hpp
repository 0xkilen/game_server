#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_rank.pb.h"
#include "proto/data_battle.pb.h"
#include "proto/data_tower.pb.h"
#include "proto/data_mansion.pb.h"
#include "proto/data_child.pb.h"
#include "proto/data_league.pb.h"
#include "proto/data_activity.pb.h"
#include "proto/data_route.pb.h"
#include "proto/config_mansion.pb.h"
#include "proto/config_quest.pb.h"
#include "utils/assert.hpp"
#include <iostream>

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {

        const pc::mansion_craft_drop& pick_craft_drop(const pc::mansion_craft_drop_array& drops, pd::actor::craft_type craft);

        inline bool operator>(const pd::rank_entity& a, const pd::rank_entity& b) {
                ASSERT(a.values_size() >= 0);
                ASSERT(a.values_size() == b.values_size());
                for (auto i = 0; i < a.values_size(); ++i) {
                        if (a.values(i) == b.values(i)) {
                                continue;
                        }
                        if (a.values(i) > b.values(i)) {
                                return true;
                        } else {
                                return false;
                        }
                }
                return false;
        }

        inline bool operator==(const pd::rank_entity& a, const pd::rank_entity& b) {
                ASSERT(a.values_size() >= 0);
                for (auto i = 0; i < a.values_size(); ++i) {
                        if (a.values(i) != b.values(i)) {
                                return false;
                        }
                }
                return true;
        }

        inline bool operator>(const pd::city_battle_damage_record& a, const pd::city_battle_damage_record& b) {
                return a.damage() > b.damage();
        }

        inline bool operator==(const pd::city_battle_damage_record& a, const pd::city_battle_damage_record& b) {
                return a.damage() == b.damage();
        }

        inline bool operator==(const pd::grid& a, const pd::grid& b) {
                if(a.row() == b.row() && a.col() == b.col()) {
                        return true;
                } else {
                        return false;
                }
        }

        inline bool operator==(const pd::voice& a, const pd::voice& b) {
                return a.id() == b.id();
        }

        inline bool operator!=(const pd::voice& a, const pd::voice& b) {
                return a.id() != b.id();
        }

        inline bool pos_in_area(const pd::position& pos, const pc::mansion_area& area) {
                return pos.x() >= area.left_down().x() && pos.y() >= area.left_down().y() &&
                        pos.x() <= area.up_right().x() && pos.y() <= area.up_right().y();
        }

        inline pd::position operator+(const pd::position& a, const pd::position& b) {
                pd::position ret;
                ret.set_x(a.x() + b.x());
                ret.set_y(a.y() + b.y());
                return ret;
        }

        inline pd::position operator*(const pd::position& a, double scale) {
                pd::position ret;
                ret.set_x(a.x() * scale);
                ret.set_y(a.y() * scale);
                return ret;
        }

        inline pc::mansion_area operator+(const pc::mansion_area& a, const pd::position& b) {
                pc::mansion_area ret;
                ret.mutable_left_down()->set_x(a.left_down().x() + b.x());
                ret.mutable_left_down()->set_y(a.left_down().y() + b.y());
                ret.mutable_up_right()->set_x(a.up_right().x() + b.x());
                ret.mutable_up_right()->set_y(a.up_right().y() + b.y());
                return ret;
        }

        inline pc::mansion_area operator*(const pc::mansion_area& a, double scale) {
                pc::mansion_area ret;
                ret.mutable_left_down()->set_x(a.left_down().x());
                ret.mutable_left_down()->set_y(a.left_down().y());
                ret.mutable_up_right()->set_x(a.up_right().x() * scale);
                ret.mutable_up_right()->set_y(a.up_right().y() * scale);
                return ret;
        }

        inline bool operator!=(const pd::position& a, const pd::position& b){
                return a.x() != b.x() || a.y() != b.y();
        }

        inline bool areas_overlapped(const pc::mansion_area& a, const pc::mansion_area& b) {
                if (a.left_down().x() > b.up_right().x()) {
                        return false;
                }
                if (a.up_right().x() < b.left_down().x()) {
                        return false;
                }
                if (a.left_down().y() > b.up_right().y()) {
                        return false;
                }
                if (a.up_right().y() < b.up_right().y()) {
                        return false;
                }
                return true;
        }

        inline bool gid_array_has_duplicate(const pd::gid_array& gids) {
                set<uint64_t> exists;
                for (auto i : gids.gids()) {
                        if (exists.count(i) > 0) {
                                return true;
                        }
                        exists.insert(i);
                }
                return false;
        }

        inline bool operator<(const pd::tower_record& a, const pd::tower_record& b) {
                if (a.role_level() < b.role_level()) {
                        return true;
                } else if (a.role_level() == b.role_level()) {
                        if (a.zhanli() < b.zhanli()) {
                                return true;
                        } else {
                                return false;
                        }
                } else {
                        return false;
                }
        }

        inline bool operator>(const pd::tower_record& a, const pd::tower_record& b) {
                if (a.role_level() > b.role_level()) {
                        return true;
                } else if (a.role_level() == b.role_level()) {
                        if (a.zhanli() > b.zhanli()) {
                                return true;
                        } else {
                                return false;
                        }
                } else {
                        return false;
                }
        }

        inline bool operator>(pd::child_phase a, pd::child_phase b) {
                return static_cast<int>(a) > static_cast<int>(b);
        }

        inline bool operator<(pd::child_phase a, pd::child_phase b) {
                return static_cast<int>(a) < static_cast<int>(b);
        }

        void merge_mansion_furniture(pd::mansion_all_furniture& a, const pd::mansion_all_furniture& b);

        inline bool operator!=(const pd::activity_mgr_prize_wheel& a, const pd::activity_mgr_prize_wheel& b) {
                return a.start_day() != b.start_day() ||
                        a.duration() != b.duration() ||
                        a.pttid() != b.pttid();
        }

        inline bool operator!=(const pd::ipport& a, const pd::ipport& b) {
                return a.ip() != b.ip() || a.port() != b.port();
        }

        inline bool operator!=(const pd::route_info& a, const pd::route_info& b) {
                return a.scened_ipport() != b.scened_ipport() || a.gmd_ipport() != b.gmd_ipport();
        }

        inline bool operator!=(const pd::route_client& a, const pd::route_client& b) {
                return a.type() != b.type() || a.id() != b.id() || a.route_info() != b.route_info();
        }

        inline bool operator!=(const pd::route_table& a, const pd::route_table& b) {
                if (a.clients_size() != b.clients_size()) {
                        return true;
                }
                for (auto i = 0; i < a.clients_size(); ++i) {
                        if (a.clients(i) != b.clients(i)) {
                                return true;
                        }
                }

                return false;
        }

}
