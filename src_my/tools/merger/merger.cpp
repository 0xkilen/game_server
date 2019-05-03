// #include "utils/service_thread.hpp"
// #include "utils/log.hpp"
// #include "utils/assert.hpp"
// #include "db/connector.hpp"
// #include "proto/data_rank.pb.h"
// #include "utils/proto_utils.hpp"
// #include <boost/program_options.hpp>
// #include <string>
// #include <iostream>
// #include <list>
// #include <set>

// using namespace std;
// using namespace nora;
// namespace bpo = boost::program_options;

// set<string> to_rolenames;
// shared_ptr<db::connector> from_db;
// shared_ptr<db::connector> to_db;
// uint32_t adding_count = 0u;
// shared_ptr<service_thread> main_st;
// int page_size = 50;
// int cur_page = 0;
// map<pd::rank_type, list<pd::rank_entity>> ranks_;
// void on_db_get_roles_done(const shared_ptr<db::message>& db_msg);
// void on_db_get_fiefs_done(const shared_ptr<db::message>& db_msg);

// void merge_roles() {
//         page_size = 50;
//         adding_count = 0;
//         auto db_msg = make_shared<db::message>("get_roles",
//                                                db::message::req_type::rt_select,
//                                                [] (const shared_ptr<db::message>& msg) {
//                                                        on_db_get_roles_done(msg);
//                                                });
//         db_msg->push_param(cur_page * page_size);
//         db_msg->push_param((cur_page + 1) * page_size);
//         from_db->push_message(db_msg, main_st);
//         cur_page += 1;
// }

// void merge_fiefs() {
//         page_size = 50;
//         adding_count = 0;
//         auto db_msg = make_shared<db::message>("get_fiefs",
//                                                db::message::req_type::rt_select,
//                                                [] (const shared_ptr<db::message>& msg) {
//                                                        on_db_get_fiefs_done(msg);
//                                                });
//         db_msg->push_param(cur_page * page_size);
//         db_msg->push_param((cur_page + 1) * page_size);
//         from_db->push_message(db_msg, main_st);
//         cur_page += 1;
// }

// void merge_gladiators() {
//         page_size = 50;
//         adding_count = 0;
//         auto db_msg = make_shared<db::message>("get_gladiators",
//                                                db::message::req_type::rt_select,
//                                                [] (const shared_ptr<db::message>& msg) {
//                                                        handle_finished_db_msg(msg);
//                                                });
//         from_db->push_message(db_msg, main_st);
//         cur_page += 1;
// }

// void merge_mansions() {
//         page_size = 50;
//         adding_count = 0;
//         auto db_msg = make_shared<db::message>("get_mansions",
//                                                db::message::req_type::rt_select,
//                                                [] (const shared_ptr<db::message>& msg) {
//                                                        handle_finished_db_msg(msg);
//                                                });
//         db_msg->push_param(cur_page * page_size);
//         db_msg->push_param((cur_page + 1) * page_size);
//         from_db->push_message(db_msg, main_st);
//         cur_page += 1;
// }

// void merge_mails() {
//         page_size = 50;
//         adding_count = 0;
//         auto db_msg = make_shared<db::message>("get_mails",
//                                                db::message::req_type::rt_select,
//                                                [] (const shared_ptr<db::message>& msg) {
//                                                        handle_finished_db_msg(msg);
//                                                });
//         db_msg->push_param(cur_page * page_size);
//         db_msg->push_param((cur_page + 1) * page_size);
//         from_db->push_message(db_msg, main_st);
//         cur_page += 1;
// }

// void merge_notices() {
//         page_size = 50;
//         adding_count = 0;
//         auto db_msg = make_shared<db::message>("get_notices",
//                                                db::message::req_type::rt_select,
//                                                [] (const shared_ptr<db::message>& msg) {
//                                                        handle_finished_db_msg(msg);
//                                                });
//         db_msg->push_param(cur_page * page_size);
//         db_msg->push_param((cur_page + 1) * page_size);
//         from_db->push_message(db_msg, main_st);
//         cur_page += 1;
// }

// void merge_leagues() {
//         adding_count = 0;
//         auto db_msg = make_shared<db::message>("get_leagues",
//                                                db::message::req_type::rt_select,
//                                                [] (const shared_ptr<db::message>& msg) {
//                                                        handle_finished_db_msg(msg);
//                                                });
//         from_db->push_message(db_msg, main_st);
// }

// void merge_rank() {
//         adding_count = 0;
//         auto db_msg = make_shared<db::message>("get_ranks",
//                                                db::message::req_type::rt_select,
//                                                [] (const shared_ptr<db::message>& msg) {
//                                                        handle_finished_db_msg(msg);
//                                                });
//         from_db->push_message(db_msg, main_st);
//         to_db->push_message(db_msg, main_st);
// }

// void on_db_get_roles_done(const shared_ptr<db::message>& db_msg) {
//         if (db_msg->results().empty()) {
//                 cout << "merge roles done" << endl;
//                 cur_page = 0;
//                 merge_fiefs();
//         }
//         for (const auto& i : db_msg->results()) {
//                 ASSERT(i.size() == 4);
//                 adding_count += 1;
//                 auto db_msg = make_shared<db::message>("add_role",
//                                                        db::message::req_type::rt_insert,
//                                                        [] (const shared_ptr<db::message>& msg) {
//                                                                adding_count -= 1;
//                                                                if (adding_count == 0) {
//                                                                        merge_roles();
//                                                                }
//                                                        });
//                 ILOG << "process role " << username << " " << rolename << " " << gid;
//                 auto username = boost::any_cast<string>(i[0]);
//                 auto gid = boost::any_cast<uint64_t>(i[1]);
//                 auto rolename = boost::any_cast<string>(i[2]);
//                 if (to_rolenames.count(rolename) > 0) {
//                         ILOG << rolename << " already exist, use gid " << gid << " as new name";
//                         rolename = to_string(gid);
//                 }
//                 db_msg->push_param(username);
//                 db_msg->push_param(gid);
//                 db_msg->push_param(rolename);
//                 db_msg->push_param(boost::any_cast<string>(i[3]));
//                 to_db->push_message(db_msg, main_st);
//         }
// }

// void on_db_get_fiefs_done(const shared_ptr<db::message>& db_msg) {
//         if (db_msg->results().empty()) {
//                 cout << "merge fiefs done" << endl;
//                 cur_page = 0;
//                 merge_mansions();
//         }

//         for (const auto& i : db_msg->results()) {
//                 ASSERT(i.size() == 2);
//                 adding_count += 1;
//                 auto db_msg = make_shared<db::message>("add_fief",
//                                                        db::message::req_type::rt_insert,
//                                                        [] (const shared_ptr<db::message>& msg) {
//                                                                adding_count -= 1;
//                                                                if (adding_count == 0) {
//                                                                        merge_fiefs();
//                                                                }
//                                                        });
//                 auto gid = boost::any_cast<uint64_t>(i[0]);
//                 ILOG << "process fief " << gid;
//                 db_msg->push_param(gid);
//                 db_msg->push_param(boost::any_cast<string>(i[1]));
//                 to_db->push_message(db_msg, main_st);
//         }
// }

// void handle_finished_db_msg(
//         if (db_msg->procedure() == "get_roles") {

//         } else if (db_msg->procedure() == "add_role") {
//         } else if (db_msg->procedure() == "get_fiefs") {

//         } else if (db_msg->procedure() == "add_fief") {
//         } else if (db_msg->procedure() == "get_mansions") {
//                 if (db_msg->results().empty()) {
//                         cout << "merge mansions done" << endl;
//                         cur_page = 0;
//                         merge_gladiators();
//                 }

//                 for (const auto& i : db_msg->results()) {
//                         ASSERT(i.size() == 2);
//                         adding_count += 1;
//                         auto db_msg = make_shared<db::message>("add_mansion",
//                                                                db::message::req_type::rt_insert,
//                                                                [] (const shared_ptr<db::message>& msg) {
//                                                                        handle_finished_db_msg(msg);
//                                                                });
//                         db_msg->push_param(boost::any_cast<uint64_t>(i[0]));
//                         db_msg->push_param(boost::any_cast<string>(i[1]));
//                         to_db->push_message(db_msg, main_st);
//                 }
//         } else if (db_msg->procedure() == "add_mansion") {
//                 adding_count -= 1;
//                 if (adding_count == 0) {
//                         merge_mansions();
//                 }
//         } else if (db_msg->procedure() == "get_gladiators") {
//                 adding_count = db_msg->results().size();
//                 if (adding_count == 0) {
//                         cur_page = 0;
//                         merge_mails();
//                         return;
//                 }
//                 for (const auto& i : db_msg->results()) {
//                         ASSERT(i.size() == 4);
//                         auto db_msg = make_shared<db::message>("add_gladiator",
//                                                                db::message::req_type::rt_insert,
//                                                                [] (const shared_ptr<db::message>& msg) {
//                                                                        handle_finished_db_msg(msg);
//                                                                });
//                         db_msg->push_param(boost::any_cast<uint64_t>(i[0]));
//                         db_msg->push_param(boost::any_cast<string>(i[1]));
//                         db_msg->push_param(boost::any_cast<string>(i[2]));
//                         db_msg->push_param(boost::any_cast<string>(i[3]));
//                         to_db->push_message(db_msg, main_st);
//                 }
//         } else if (db_msg->procedure() == "add_gladiator") {
//                 adding_count -= 1;
//                 if (adding_count == 0) {
//                         merge_mails();;
//                 }
//         } else if (db_msg->procedure() == "get_mails") {
//                 if (db_msg->results().empty()) {
//                         cout << "merge mails done" << endl;
//                         cur_page = 0;
//                         merge_notices();
//                 }

//                 for (const auto& i : db_msg->results()) {
//                         ASSERT(i.size() == 6);
//                         adding_count += 1;
//                         auto db_msg = make_shared<db::message>("add_mail",
//                                                                db::message::req_type::rt_insert,
//                                                                [] (const shared_ptr<db::message>& msg) {
//                                                                        handle_finished_db_msg(msg);
//                                                                });
//                         db_msg->push_param(boost::any_cast<uint64_t>(i[0]));
//                         db_msg->push_param(boost::any_cast<uint64_t>(i[1]));
//                         db_msg->push_param(boost::any_cast<uint32_t>(i[2]));
//                         db_msg->push_param(boost::any_cast<uint32_t>(i[3]));
//                         db_msg->push_param(boost::any_cast<uint32_t>(i[4]));
//                         db_msg->push_param(boost::any_cast<string>(i[5]));
//                         to_db->push_message(db_msg, main_st);
//                 }
//         } else if (db_msg->procedure() == "add_mail") {
//                 adding_count -= 1;
//                 if (adding_count == 0) {
//                         merge_mails();
//                 }
//         } else if (db_msg->procedure() == "get_notices") {
//                 if (db_msg->results().empty()) {
//                         cout << "merge notices done" << endl;
//                         cur_page = 0;
//                         merge_leagues();
//                 }

//                 for (const auto& i : db_msg->results()) {
//                         ASSERT(i.size() == 4);
//                         adding_count += 1;
//                         auto db_msg = make_shared<db::message>("add_notice",
//                                                                db::message::req_type::rt_insert,
//                                                                [] (const shared_ptr<db::message>& msg) {
//                                                                        handle_finished_db_msg(msg);
//                                                                });
//                         db_msg->push_param(boost::any_cast<uint64_t>(i[0]));
//                         db_msg->push_param(boost::any_cast<uint64_t>(i[1]));
//                         db_msg->push_param(boost::any_cast<uint32_t>(i[2]));
//                         db_msg->push_param(boost::any_cast<string>(i[3]));
//                         to_db->push_message(db_msg, main_st);
//                 }
//         } else if (db_msg->procedure() == "add_notice") {
//                 adding_count -= 1;
//                 if (adding_count == 0) {
//                         merge_notices();
//                 }
//         } else if (db_msg->procedure() == "get_leagues") {
//                 adding_count = db_msg->results().size();
//                 if (adding_count == 0) {
//                         cout << "begin merge rank ..." << endl;
//                         cur_page = 0;
//                         merge_rank();
//                         return;
//                 }
//                 for (const auto& i : db_msg->results()) {
//                         ASSERT(i.size() == 2);
//                         auto db_msg = make_shared<db::message>("add_league",
//                                                                db::message::req_type::rt_insert,
//                                                                [] (const shared_ptr<db::message>& msg) {
//                                                                        handle_finished_db_msg(msg);
//                                                                });
//                         db_msg->push_param(boost::any_cast<uint64_t>(i[0]));
//                         db_msg->push_param(boost::any_cast<string>(i[1]));
//                         to_db->push_message(db_msg, main_st);
//                 }
//         } else if (db_msg->procedure() == "add_league") {
//                 adding_count -= 1;
//                 if (adding_count == 0) {
//                         cout << "merge leagues done" << endl;
//                         merge_rank();
//                 }
//         } else if (db_msg->procedure() == "get_ranks") {
//                 adding_count += 1;
//                 if (!db_msg->results().empty()) {
//                         set<pd::rank_type> rank_types = {
//                                 pd::RT_LEVEL,
//                                 pd::RT_ZHANLI,
//                                 pd::RT_RECEIVE_GIFT,
//                                 pd::RT_GIVE_GIFT,
//                                 pd::RT_MANSION_FANCY,
//                                 pd::RT_CHAOTANG,
//                         };
//                         const auto& result = db_msg->results().front();
//                         for (const auto& i : result) {
//                                 pd::rank rank;
//                                 rank.ParseFromString(boost::any_cast<string>(i));
//                                 for (const auto& j : rank.entities()) {
//                                         if (rank_types.count(rank.type()) > 0) {
//                                                 rank_types.erase(rank.type());
//                                         }
//                                         ranks_[rank.type()].push_back(j);
//                                 }
//                         }
//                         for (auto i : rank_types) {
//                                 if (ranks_.count(i) <= 0) {
//                                         ranks_[i].push_back(pd::rank_entity());
//                                 }
//                         }
//                 }

//                 if (adding_count == 2)  {
//                         if (ranks_.empty()) {
//                                 cout << "merge rank done" << endl;
//                                 main_st->stop();
//                                 return;
//                         }

//                         adding_count -= 1;
//                         auto db_msg = make_shared<db::message>("add_rank",
//                                                                db::message::req_type::rt_insert,
//                                                                [] (const shared_ptr<db::message>& msg) {
//                                                                        handle_finished_db_msg(msg);
//                                                         });
//                         ASSERT(ranks_.size() == size_t(6));
//                         for (auto& i : ranks_) {
//                                 i.second.sort(
//                                         [] (const auto& a, const auto& b) {
//                                                 return a > b;
//                                         });
//                                 cout << "add rank type: " << pd::rank_type_Name(i.first) << "\n";
//                                 pd::rank rank;
//                                 rank.set_type(i.first);
//                                 if (i.second.front().gid() != 0) {
//                                         for (const auto& j : i.second) {
//                                                 *rank.add_entities() = j;
//                                         }
//                                 }

//                                 string rank_str;
//                                 rank.SerializeToString(&rank_str);
//                                 db_msg->push_param(rank_str);
//                         }
//                         to_db->push_message(db_msg, main_st);
//                 }
//         } else if (db_msg->procedure() == "add_rank") {
//                 adding_count -= 1;
//                 if (adding_count == 0) {
//                         cout << "merge rank done" << endl;
//                         main_st->stop();
//                 }
//         }
// }

// int main(int argc, char *argv[]) {
//         bpo::options_description desc("options");
//         desc.add_options()
//                 ("help", "produce help message")
//                 ("from_ipport", bpo::value<string>(), "from db ipport, like 127.0.0.1:3306")
//                 ("from_user", bpo::value<string>(), "from db username")
//                 ("from_password", bpo::value<string>(), "from db password")
//                 ("from_database", bpo::value<string>(), "from db database")
//                 ("to_ipport", bpo::value<string>(), "to db ipport, like 127.0.0.1:3306")
//                 ("to_user", bpo::value<string>(), "to db username")
//                 ("to_password", bpo::value<string>(), "to db password")
//                 ("to_database", bpo::value<string>(), "to db database");
//         bpo::variables_map vm;
//         try {
//                 bpo::store(bpo::command_line_parser(argc, argv).options(desc).run(), vm);
//                 bpo::notify(vm);
//         } catch (const exception& e) {
//                 cerr << e.what() << endl;
//                 return 1;
//         }

//         if (vm.count("help")) {
//                 cout << desc << "\n";
//                 return 0;
//         }
//         if (!vm.count("from_ipport") || !vm.count("from_user") || !vm.count("from_password") || !vm.count("from_database")) {
//                 cout << "missing from db info" << "\n";
//                 return 1;
//         }
//         if (!vm.count("to_ipport") || !vm.count("to_user") || !vm.count("to_password") || !vm.count("to_database")) {
//                 cout << "missing to db info" << "\n";
//                 return 1;
//         }
//         auto from_ipport = vm["from_ipport"].as<string>();
//         auto from_user = vm["from_user"].as<string>();
//         auto from_password = vm["from_password"].as<string>();
//         auto from_database = vm["from_database"].as<string>();
//         auto to_ipport = vm["to_ipport"].as<string>();
//         auto to_user = vm["to_user"].as<string>();
//         auto to_password = vm["to_password"].as<string>();
//         auto to_database = vm["to_database"].as<string>();
//         if (from_ipport == to_ipport && from_database == to_database) {
//                 cout << "from and to are the same" << "\n";
//                 return 1;
//         }

//         from_db = make_shared<db::connector>(from_ipport,
//                                              from_user,
//                                              from_password,
//                                              from_database);
//         from_db->start();
//         to_db = make_shared<db::connector>(to_ipport,
//                                            to_user,
//                                            to_password,
//                                            to_database);
//         to_db->start();
//         cout << "begin merge db ..." << "\n";
//         main_st = make_shared<service_thread>("main");
//         main_st->async_call(
//                 [] {
//                         auto db_msg = make_shared<db::message>("load_gid_and_rolename",
//                                                                db::message::req_type::rt_select,
//                                                                [] (const auto& msg) {
//                                                                        auto& results = msg->results();
//                                                                        for (const auto& i : results) {
//                                                                                ASSERT(i.size() == 2);
//                                                                                auto rolename = boost::any_cast<string>(i[1]);
//                                                                                to_rolenames.insert(rolename);
//                                                                        }
//                                                                        cout << "begin merge roles ..." << "\n";
//                                                                        merge_roles();
//                                                                });
//                         to_db->push_message(db_msg, main_st);
//                 });

//         main_st->run();

//         from_db->stop();
//         to_db->stop();

//         return 0;
// }
