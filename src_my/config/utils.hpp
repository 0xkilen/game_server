#pragma once

#include "utils/ptts.hpp"
#include "proto/data_condition.pb.h"
#include "proto/data_event.pb.h"
#include "proto/data_resource.pb.h"
#include "proto/data_condevent.pb.h"
#include <boost/filesystem.hpp>
#include <string>
#include <set>

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;
namespace bfs = boost::filesystem;

#define PTTS_SET_FUNCS(type) nora::config::type##_ptts_set_funcs()

#define PTTS_LOAD(type) CONFIG_ILOG << "load config " << #type;         \
        if (bfs::exists(bfs::path(config_path + "/" + #type + "_table.csv")) || bfs::exists(bfs::path(config_path + "/" + #type + "_table_1.csv"))) { \
                nora::config::type##_ptts_instance().load_csv_files(#type); \
        } else {                                                        \
                CONFIG_ELOG << "cannot find config file for " << #type; \
        }

#define PTTS_MVP(type) CONFIG_ILOG << "check modify and verify config " << #type; \
        nora::config::type##_ptts_instance().check();                   \
        nora::config::type##_ptts_instance().modify();                  \
        nora::config::type##_ptts_instance().verify();                  \
        nora::config::type##_ptts_instance().enrich();                  \
        CONFIG_ILOG << "print config " << #type;                        \
        nora::config::type##_ptts_instance().print();                   \

#define PTTS_WRITE_ALL(type) {                                          \
                pc::type##_file f;                                      \
                nora::config::type##_ptts_instance().write_all_file(#type, &f); \
        }

#define PTTS_GET(type, pttid) nora::config::type##_ptts_instance().get_ptt(static_cast<uint32_t>(pttid))
#define PTTS_GET_COPY(type, pttid) nora::config::type##_ptts_instance().get_ptt_copy(static_cast<uint32_t>(pttid))
#define PTTS_GET_ALL(type) nora::config::type##_ptts_instance().get_ptt_all()
#define PTTS_GET_ALL_COPY(type) nora::config::type##_ptts_instance().get_ptt_all_copy()
#define PTTS_HAS(type, pttid) nora::config::type##_ptts_instance().has_ptt(static_cast<uint32_t>(pttid))
/*#define SPINE_PTTS_GET(part, pttid) nora::config::spine_ptts_get(part, pttid)
#define SPINE_PTTS_HAS(part, pttid) nora::config::spine_ptts_has(part, pttid)
#define HUANZHUANG_PTTS_GET(part, pttid) nora::config::huanzhuang_ptts_get(part, pttid)
#define HUANZHUANG_PTTS_HAS(part, pttid) nora::config::huanzhuang_ptts_has(part, pttid)
*/
namespace nora {
        namespace config {

                bool check_conditions(const pd::condition_array& ca);
                bool verify_conditions(const pd::condition_array& ca);

                bool check_events(const pd::event_array& ea);
                bool verify_events(const pd::event_array& ea);

                bool check_condevents(const pd::condevent_array& cea);
                bool verify_condevents(const pd::condevent_array& cea);

                void modify_events_by_conditions(const pd::condition_array& ca, pd::event_array& ea);
                void modify_conditions_by_events(const pd::event_array& ea, pd::condition_array& ca);

                void load_config();
                void check_config();
                void load_dirty_word();

                void write_all();

                set<uint32_t> quest_to_all_quests(uint32_t quest_pttid);
        }
}
