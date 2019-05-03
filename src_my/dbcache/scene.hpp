#pragma once

#include "dbc_object.hpp"
#include "dbc_object_mgr.hpp"
#include "dbc_object_single.hpp"
#include "net/conn.hpp"
#include "net/stream.hpp"
#include "net/server.hpp"
#include "db/message.hpp"
#include "db/connector.hpp"
#include "proto/processor.hpp"
#include "proto/ss_base.pb.h"
#include "proto/data_huanzhuang_pvp.pb.h"
#include "proto/data_player.pb.h"
#include "proto/data_league.pb.h"
#include "proto/data_arena.pb.h"
#include "proto/data_mansion.pb.h"
#include "proto/data_fief.pb.h"
#include "proto/data_base.pb.h"
#include "proto/data_rank.pb.h"
#include "proto/data_guanpin.pb.h"
#include "proto/data_marriage_data.pb.h"
#include "proto/data_child.pb.h"
#include "proto/data_activity.pb.h"
#include <map>
#include <queue>
#include <memory>
#include <boost/core/noncopyable.hpp>
#include <boost/asio.hpp>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;
namespace ba = boost::asio;

namespace nora {
        class service_thread;
        namespace dbcache {

                class scene : public enable_shared_from_this<scene>,
                              public proto::processor<scene, ps::base>,
                              private boost::noncopyable {
                public:
                        scene(const string& name, const shared_ptr<service_thread>& st);
                        void start(const string& ip, unsigned short port);
                        void try_stop();
                        void stop();
                        void process_msg(const shared_ptr<net::recvstream>& msg);
                        friend ostream& operator<<(ostream& os, const scene& s);
                        function<void()> stop_cb_;
                private:
                        void process_msgs();
                        void process_load_ranks_req(const ps::base *msg);
                        void process_create_role_req(const ps::base *msg);
                        void process_find_role_req(const ps::base *msg);
                        void process_find_role_by_gid_req(const ps::base *msg);
                        void process_update_role_req(const ps::base *msg);
                        void process_create_fief_req(const ps::base *msg);
                        void process_find_fief_req(const ps::base *msg);
                        void process_update_fief_req(const ps::base *msg);
                        void process_create_mansion_req(const ps::base *msg);
                        void process_find_mansion_req(const ps::base *msg);
                        void process_update_mansion_req(const ps::base *msg);
                        void process_load_huanzhuang_pvp_req(const ps::base *msg);
                        void process_save_huanzhuang_pvp_req(const ps::base *msg);
                        void process_load_marriage_req(const ps::base *msg);
                        void process_save_marriage_req(const ps::base *msg);
                        void process_save_mansion_mgr_req(const ps::base *msg);
                        void process_load_mansion_mgr_req(const ps::base *msg);
                        void process_create_league_req(const ps::base *msg);
                        void process_get_league_req(const ps::base *msg);
                        void process_update_league_req(const ps::base *msg);
                        void process_get_leagues_req(const ps::base *msg);
                        void process_get_league_separations_req(const ps::base *msg);
                        void process_delete_league_req(const ps::base *msg);
                        void process_add_cities_req(const ps::base *msg);
                        void process_get_city_req(const ps::base *msg);
                        void process_update_city_req(const ps::base *msg);
                        void process_update_rank_req(const ps::base *msg);
                        void process_get_rank_req(const ps::base *msg);
                        void process_get_arena_rank_req(const ps::base *msg);
                        void process_add_gladiators_req(const ps::base *msg);
                        void process_get_gladiator_req(const ps::base *msg);
                        void process_update_arena_rank_req(const ps::base *msg);
                        void process_update_gladiator_req(const ps::base *msg);
                        void process_load_guanpin_req(const ps::base *msg);
                        void process_guanpin_add_guan_req(const ps::base *msg);
                        void process_guanpin_get_guan_req(const ps::base *msg);
                        void process_save_guanpin_req(const ps::base *msg);
                        void process_guanpin_update_guan_req(const ps::base *msg);
                        void process_load_league_war_req(const ps::base *msg);
                        void process_save_league_war_req(const ps::base *msg);
                        void process_find_child_req(const ps::base *msg);
                        void process_add_child_req(const ps::base *msg);
                        void process_save_child_req(const ps::base *msg);
                        void process_load_child_mgr_req(const ps::base *msg);
                        void process_save_child_mgr_req(const ps::base *msg);
                        void process_load_activity_mgr_req(const ps::base *msg);
                        void process_save_activity_mgr_req(const ps::base *msg);
                        void process_load_league_mgr_req(const ps::base *msg);
                        void process_save_league_mgr_req(const ps::base *msg);
                        void process_update_whole_rank_req(const ps::base *msg);

                        void on_db_add_role_done(const string& username, const pd::role& data, bool already_exist);
                        void on_db_find_role_done(const string& username, const vector<vector<boost::any>>& results, const pd::db_client_data& db_client_data);
                        void on_db_find_role_by_gid_done(uint64_t gid, const vector<vector<boost::any>>& results);
                        void on_db_update_role_done();
                        void on_db_add_league_done(const pd::league& data, const pd::league_separation& separation, bool already_exist);
                        void on_db_get_league_done(uint64_t gid, const vector<vector<boost::any>>& results);
                        void on_db_get_leagues_done(const vector<vector<boost::any>>& results);
                        void on_db_get_league_separations_done(const vector<vector<boost::any>>& results);
                        void on_db_load_ranks_done(const vector<vector<boost::any>>& results);
                        void on_db_load_guanpin_done(const vector<vector<boost::any>>& results);
                        void on_db_find_guan_done(uint64_t gid, const vector<vector<boost::any>>& results);
                        void on_db_load_league_war_done(const vector<vector<boost::any>>& results);
                        void send_to_scene(const ps::base& msg);

                        void update_role(uint64_t gid, const pd::role& data);
                        void clean_roles();
                        void clean_leagues();

                        void try_save_role();
                        void update_league(uint64_t gid, const pd::league_updatable& updatable);
                        void try_save_league();

                        void add_stop_timer();
                        bool can_stop() const;

                        const string name_;
                        shared_ptr<service_thread> st_;
                        vector<shared_ptr<service_thread>> parse_sts_;
                        shared_ptr<service_thread> serialize_st_;
                        uint32_t next_parse_st_ = 0;
                        shared_ptr<net::server<net::CONN>> nsv_;
                        shared_ptr<net::CONN> conn_;
                        enum gamedb_table {
                                GDT_ROLE,
                                GDT_RANK,
                                GDT_LEAGUE,
                                GDT_COUNT,
                        };
                        map<gamedb_table, shared_ptr<db::connector>> gamedbs_;

                        set<string> finding_roles_;
                        set<uint64_t> finding_by_gid_roles_;
                        map<string, unique_ptr<dbc_object<pd::role>>> username2role_;
                        map<uint64_t, string> gid2username_;
                        free_list role_free_list_;
                        free_list role_dirty_list_;
                        bool saving_role_ = false;
                        bool cleaning_roles_ = false;

                        shared_ptr<dbc_object_mgr<pd::mansion>> mansions_;
                        shared_ptr<dbc_object_single<pd::mansion_mgr>> mansion_mgr_;

                        shared_ptr<dbc_object_single<pd::huanzhuang_pvp>> huanzhuang_pvp_;

                        shared_ptr<dbc_object_single<pd::marriage_data>> marriage_;

                        bool getting_all_leagues = false;
                        bool getting_all_league_separations_ = false;

                        map<uint64_t, uint32_t> leauge_operate_;

                        free_list league_free_list_;
                        free_list league_dirty_list_;
                        bool saving_league_ = false;
                        bool cleaning_leagues_ = false;

                        shared_ptr<dbc_object_mgr<pd::rank>> rank_;
                        uint32_t add_rank_size_ = 0;

                        shared_ptr<dbc_object_single<pd::arena_rank>> arena_rank_;
                        shared_ptr<dbc_object_mgr<pd::gladiator>> gladiators_;

                        shared_ptr<dbc_object_single<pd::guanpin>> guanpin_;
                        shared_ptr<dbc_object_mgr<pd::guan>> guans_;

                        shared_ptr<dbc_object_mgr<pd::league>> leagues_;
                        shared_ptr<dbc_object_mgr<pd::league_separation>> league_separations_;
                        shared_ptr<dbc_object_mgr<pd::city>> cities_;

                        shared_ptr<dbc_object_single<pd::league_war>> league_war_;

                        shared_ptr<dbc_object_single<pd::child_mgr>> child_mgr_;
                        shared_ptr<dbc_object_mgr<pd::child>> children_;

                        shared_ptr<dbc_object_mgr<pd::fief>> fiefs_;

                        shared_ptr<dbc_object_single<pd::activity_mgr>> activity_mgr_;
                        
                        shared_ptr<dbc_object_single<pd::league_mgr>> league_mgr_;

                        uint32_t msg_id_ = 1;
                        mutable mutex msgs_lock_;
                        uint32_t process_msg_id_ = 1;
                        map<uint32_t, ps::base> msgs_;

                        bool stop_ = false;
                };

        }
}
