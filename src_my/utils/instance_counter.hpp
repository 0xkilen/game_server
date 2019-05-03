#pragma once

#include "log.hpp"
#include <mutex>
#include <map>

using namespace std;

namespace nora {

        enum instance_class_type {
                ICT_CPLAYER = 0,
                ICT_PLAYER = 10,
                ICT_INIT_PLAYER = 11,
                ICT_CHECK_LOGIN_PLAYER = 12,
                ICT_FIND_ROLE_PLAYER = 13,
                ICT_FETCH_DATA_PLAYER = 14,
                ICT_CLIENT_CREATE_ROLE_PLAYER = 15,
                ICT_CREATE_ROLE_PLAYER = 16,
                ICT_PLAYING_PLAYER = 17,
                ICT_QUIT_PLAYER = 18,
                ICT_MANSION = 30,
                ICT_FIEF = 40,
                ICT_ROOM = 50,
                ICT_LEAGUE = 60,
                ICT_LEAGUE_MEMBER = 61,
                ICT_LEAGUE_ONLINE_ROLE = 62,
                ICT_LEAGUE_WAR = 63,
                ICT_ACTOR = 70,
                ICT_ITEM = 80,
                ICT_EQUIPMENT = 90,
                ICT_WS_CONN = 100,
                ICT_CONN = 101,
                ICT_HTTP_CONN = 102,
                ICT_DB_MESSAGE = 110,
                ICT_GLADIATOR = 120,
                ICT_HUANZHUANG_PVP_ROLE = 130,
                ICT_GUAN = 140,
                ICT_BATTLE = 150,
                ICT_BATTLE_ACTOR = 151,
                ICT_BATTLE_BUFF = 152,
                ICT_BATTLE_TEAM = 153,
                ICT_BATTLE_SKILL = 154,
                ICT_GONGDOU = 160,
                ICT_ROLE = 170,
                ICT_HUANZHUANG = 180,
                ICT_CHILD = 190,

                ICT_DBC_ROLE = 500,
                ICT_DBC_MANSION = 510,
                ICT_DBC_FIEF = 520,
                ICT_DBC_LEAGUE = 530,
                ICT_DBC_LEAGUE_SEPARATION = 531,
                ICT_DBC_CITY = 532,
                ICT_DBC_GLADIATOR = 540,
                ICT_DBC_GUAN = 550,
                ICT_DBC_CHILD = 560,
                ICT_DBC_RANK = 570,

                ICT_IGNORE = 9999
        };

        class instance_counter {
        public:
                static instance_counter& instance() {
                        static instance_counter inst;
                        return inst;
                }
                void add(instance_class_type type, int count = 1) {
                        lock_guard<mutex> lock(lock_);
                        if (!live_ || type == ICT_IGNORE) {
                                return;
                        }
                        type2count_[type] += count;
                }
                void dec(instance_class_type type, int count = 1) {
                        lock_guard<mutex> lock(lock_);
                        if (!live_ || type == ICT_IGNORE) {
                                return;
                        }
                        type2count_[type] -= count;
                }
                void print() {
                        lock_guard<mutex> lock(lock_);
                        if (!live_) {
                                return;
                        }
                        stringstream ss;
                        for (const auto& i : type2count_) {
                                ss << setw(30) << type2name_[i.first] << setw(10) << i.second << "\n";
                        }
                        ILOG << "\n====== class instance counter\n" << ss.str();
                }
                instance_counter() {
                        live_ = true;
                }
                ~instance_counter() {
                        live_ = false;
                }
        private:
                static mutex lock_;
                static bool live_;
                map<instance_class_type, int> type2count_;
                map<instance_class_type, string> type2name_ = {
                        { ICT_CPLAYER, "cplayer" },
                        { ICT_PLAYER, "player" },
                        { ICT_INIT_PLAYER, "init_player" },
                        { ICT_CHECK_LOGIN_PLAYER, "check_login_player" },
                        { ICT_FIND_ROLE_PLAYER, "find_role_player" },
                        { ICT_FETCH_DATA_PLAYER, "fetch_data_player" },
                        { ICT_CLIENT_CREATE_ROLE_PLAYER, "client_create_role_player" },
                        { ICT_CREATE_ROLE_PLAYER, "create_role_player" },
                        { ICT_PLAYING_PLAYER, "playing_player" },
                        { ICT_QUIT_PLAYER, "quit_player" },
                        { ICT_MANSION, "mansion" },
                        { ICT_FIEF, "fief" },
                        { ICT_ROOM, "room" },
                        { ICT_LEAGUE, "league" },
                        { ICT_LEAGUE_MEMBER, "league_member" },
                        { ICT_ACTOR, "actor" },
                        { ICT_ITEM, "item" },
                        { ICT_EQUIPMENT, "equipment" },
                        { ICT_WS_CONN, "ws_conn" },
                        { ICT_CONN, "conn" },
                        { ICT_HTTP_CONN, "http_conn" },
                        { ICT_DB_MESSAGE, "db_message" },
                        { ICT_GLADIATOR, "gladiator" },
                        { ICT_BATTLE, "battle" },
                        { ICT_BATTLE_ACTOR, "battle_actor" },
                        { ICT_BATTLE_BUFF, "battle_buff" },
                        { ICT_BATTLE_TEAM, "battle_team" },
                        { ICT_BATTLE_SKILL, "battle_skill" },
                        { ICT_HUANZHUANG_PVP_ROLE, "huanzhuang_pvp_role" },
                        { ICT_GUAN, "guan" },
                        { ICT_GONGDOU, "gongdou" },
                        { ICT_ROLE, "role" },
                        { ICT_HUANZHUANG, "huanzhuang" },
                        { ICT_CHILD, "child" },
                        { ICT_LEAGUE_ONLINE_ROLE, "league_online_role" },
                        { ICT_LEAGUE_WAR, "league_war" },
                        { ICT_DBC_ROLE, "dbc_role" },
                        { ICT_DBC_MANSION, "dbc_mansion" },
                        { ICT_DBC_FIEF, "dbc_fief" },
                        { ICT_DBC_LEAGUE, "dbc_league" },
                        { ICT_DBC_CITY, "dbc_city" },
                        { ICT_DBC_GLADIATOR, "dbc_gladiator" },
                        { ICT_DBC_GUAN, "dbc_guan" },
                        { ICT_DBC_CHILD, "dbc_child" },
                        { ICT_DBC_RANK, "dbc_rank" }
                };
        };

        class instance_countee {
        public:
                instance_countee(instance_class_type type) : type_(type) {
                        instance_counter::instance().add(type_);
                }
                instance_countee(const instance_countee& other) : type_(other.type_) {
                        instance_counter::instance().add(type_);
                }
                ~instance_countee() {
                        instance_counter::instance().dec(type_);
                }
        private:
                instance_class_type type_;
        };

}
