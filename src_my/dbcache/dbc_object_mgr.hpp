#pragma once

#include "dbcache_logic.hpp"
#include "utils/free_list.hpp"
#include "utils/assert.hpp"
#include "dbcache_logic.hpp"
#include "log.hpp"
#include "db/message.hpp"
#include "db/connector.hpp"
#include "config/options_ptts.hpp"
#include <boost/any.hpp>
#include <vector>
#include <memory>

using namespace std;

namespace nora {
        namespace dbcache {

                template <typename T>
                class dbc_object_mgr : public enable_shared_from_this<dbc_object_mgr<T>> {
                public:
                        dbc_object_mgr(const string& name,
                                       const shared_ptr<service_thread>& st,
                                       instance_class_type ic_type,
                                       uint32_t max_count) :
                                name_(name),
                                st_(st),
                                ic_type_(ic_type),
                                max_count_(max_count) {
                                auto ptt = PTTS_GET_COPY(options, 1u);
                                db_connector_ = make_shared<db::connector>(ptt.game_db().ipport(),
                                                                           ptt.game_db().user(),
                                                                           ptt.game_db().password(),
                                                                           ptt.game_db().database());
                                db_connector_->start();
                        }
                        void add(const T& data, const function<void()>& cb) {
                                ASSERT(st_->check_in_thread());
                                auto gid = data.gid();
                                ASSERT(objects_.count(gid) == 0);
                                objects_.emplace(gid, make_unique<dbc_object<T>>(ic_type_));
                                auto& o = objects_.at(gid);
                                o->set(data);
                                free_list_.push(gid);

                                DBCACHE_DLOG << *this << " add " << gid;
                                string data_str;
                                data.SerializeToString(&data_str);
                                auto db_msg = make_shared<db::message>(string("add_") + name_,
                                                                       db::message::req_type::rt_insert,
                                                                       [this, self = this->shared_from_this(), gid, data, cb](const auto& msg) {
                                                                               ASSERT(st_->check_in_thread());
                                                                               ASSERT(!msg->update_got_duplicate());

                                                                               DBCACHE_DLOG << *this << " add " << gid << " done";
                                                                               if (cb) {
                                                                                       cb();
                                                                               }

                                                                               this->clean();
                                                                       });
                                db_msg->push_param(gid);
                                db_msg->push_param(data_str);
                                ASSERT(db_connector_);
                                db_connector_->push_message(db_msg, st_);
                        }
                        void add(const vector<T>& data_vec, const function<void(uint32_t)>& cb) {
                                ASSERT(st_->check_in_thread());
                                for (auto i = 0u; i < data_vec.size(); ++i) {
                                        add(data_vec[i],
                                            [this, self = this->shared_from_this(), count = data_vec.size(), cur_count = i + 1, cb] {
                                                    if (cb && cur_count == count) {
                                                            cb(count);
                                                    }
                                            });
                                }
                        }
                        void get(uint64_t gid, const function<void(pd::result, const T&)>& cb) {
                                ASSERT(st_->check_in_thread());
                                ASSERT(cb);
                                if (objects_.count(gid) > 0) {
                                        cb(pd::OK, objects_.at(gid)->data());
                                        free_list_.touch(gid);
                                } else {
                                        dbc_call_funcs_[gid].push_back(cb);
                                        if (finding_gids_.count(gid) <= 0) {
                                                finding_gids_.insert(gid);
                                                auto db_msg = make_shared<db::message>(string("find_") + name_,
                                                                                       db::message::req_type::rt_select,
                                                                                       [this, self = this->shared_from_this(), gid] (const auto& msg) {
                                                                                                auto result = this->on_db_find_done(gid, msg->results());
                                                                                                ASSERT(this->dbc_call_funcs_.count(gid) > 0);
                                                                                                try {
                                                                                                        for (const auto& i : this->dbc_call_funcs_[gid]) {
                                                                                                                if (result == pd::OK) {
                                                                                                                       i(result, objects_.at(gid)->data());
                                                                                                                } else {
                                                                                                                       i(result, T());
                                                                                                                }
                                                                                                        }
                                                                                                } catch (...) {
                                                                                                        DBCACHE_ELOG << "dbc object mgr call func when find done throw";
                                                                                                }
                                                                                                this->dbc_call_funcs_.erase(gid);
                                                                                       });

                                                db_msg->push_param(gid);
                                                ASSERT(db_connector_);
                                                db_connector_->push_message(db_msg, st_);
                                        }
                                }
                        }

                        void update(uint64_t gid, const T& data) {
                                ASSERT(st_->check_in_thread());
                                get(gid,
                                    [this, self = this->shared_from_this(), gid, data] (auto result, const auto& data) {
                                            ASSERT(st_->check_in_thread());
                                            ASSERT(result == pd::OK);
                                            ASSERT(objects_.count(gid) > 0);

                                            auto& o = objects_.at(gid);
                                            o->update(data);

                                            free_list_.touch(gid);

                                            if (dirty_list_.has(gid)) {
                                                    if (o->dirty() < 8) {
                                                            dirty_list_.touch(gid);
                                                    }
                                            } else {
                                                    dirty_list_.push(gid);
                                            }

                                            this->try_update();
                                    });
                        }
                        void update(uint64_t gid, const function<void(T&)>& update_func) {
                                ASSERT(st_->check_in_thread());
                                ASSERT(update_func);
                                get(gid,
                                    [this, self = this->shared_from_this(), gid, update_func] (auto result, const auto& data) {
                                            ASSERT(st_->check_in_thread());
                                            ASSERT(result == pd::OK);
                                            ASSERT(objects_.count(gid) > 0);

                                            auto& o = objects_.at(gid);
                                            o->update(update_func);

                                            free_list_.touch(gid);

                                            if (dirty_list_.has(gid)) {
                                                    if (o->dirty() < 8) {
                                                            dirty_list_.touch(gid);
                                                    }
                                            } else {
                                                    dirty_list_.push(gid);
                                            }

                                            this->try_update();
                                    });
                        }

                        void remove (uint64_t gid) {
                                ASSERT(st_->check_in_thread());
                                if (objects_.count(gid) > 0) {
                                        objects_.erase(gid);
                                }

                                if (free_list_.has(gid)) {
                                        free_list_.remove(gid);
                                }

                                if (dirty_list_.has(gid)) {
                                        dirty_list_.remove(gid);
                                }
                                auto db_msg = make_shared<db::message>(string("delete_") + name_,
                                                                        db::message::req_type::rt_select,
                                                                        [this, self = this->shared_from_this(), gid] (const auto& msg) {
                                                                                DBCACHE_DLOG << name_ << gid << " done";
                                                                        });

                                db_msg->push_param(gid);
                                ASSERT(db_connector_);
                                db_connector_->push_message(db_msg, st_);                                
                        }

                        void clean() {
                                if (cleaning_) {
                                        return;
                                }

                                cleaning_ = true;
                                ADD_TIMER(
                                        st_,
                                        ([this, self = this->shared_from_this()] (auto canceled, const auto& timer) {
                                                cleaning_ = false;
                                                if (!canceled) {
                                                        auto max_try = free_list_.size();
                                                        while (objects_.size() > max_count_ && max_try > 0) {
                                                                auto gid = free_list_.pop();
                                                                ASSERT(objects_.count(gid) > 0);
                                                                if (objects_.at(gid)->dirty() > 0) {
                                                                        free_list_.push(gid);
                                                                } else {
                                                                        objects_.erase(gid);
                                                                }
                                                                max_try -= 1;
                                                        }
                                                }
                                        }), 3s);
                        }
                        bool can_stop() {
                                return dirty_list_.empty() && !updating_;
                        }
                        friend ostream& operator<<(ostream& os, const dbc_object_mgr<T>& dom) {
                                return os << dom.name_ << "_mgr";
                        }
                private:
                        pd::result on_db_find_done(uint64_t gid, const vector<vector<boost::any>>& results) {
                                ASSERT(st_->check_in_thread());
                                ASSERT(results.size() <= 1);

                                finding_gids_.erase(gid);
                                if (results.size() > 0) {
                                        const auto& result = results.front();
                                        objects_.emplace(gid, make_unique<dbc_object<T>>(ic_type_));
                                        auto& o = objects_.at(gid);
                                        o->parse(boost::any_cast<string>(result[0]));

                                        if (free_list_.has(gid)) {
                                                free_list_.touch(gid);
                                        } else {
                                                free_list_.push(gid);
                                        }
                                        clean();
                                        return pd::OK;
                                } else {
                                        return pd::NOT_FOUND;
                                }
                        }

                        void try_update() {
                                if (dirty_list_.empty() || updating_) {
                                        return;
                                }

                                auto gid = dirty_list_.pop();
                                auto& o = objects_.at(gid);
                                DBCACHE_DLOG << *this << " update " << name_ << " " << gid;
                                const auto& data = o->data();
                                auto db_msg = make_shared<db::message>(string("update_") + name_,
                                                                       db::message::req_type::rt_update,
                                                                       [this, self = this->shared_from_this()] (const auto& msg) {
                                                                               updating_ = false;
                                                                               this->try_update();
                                                                       });

                                db_msg->push_param(gid);
                                string data_str;
                                data.SerializeToString(&data_str);
                                db_msg->push_param(data_str);
                                ASSERT(db_connector_);
                                db_connector_->push_message(db_msg, st_);

                                updating_ = true;
                                o->reset_dirty();
                        }

                        string name_;
                        shared_ptr<service_thread> st_;
                        instance_class_type ic_type_;
                        set<uint64_t> finding_gids_;
                        map<uint64_t, unique_ptr<dbc_object<T>>> objects_;
                        free_list free_list_;
                        free_list dirty_list_;
                        bool updating_ = false;
                        bool cleaning_ = false;
                        shared_ptr<db::connector> db_connector_;
                        uint32_t max_count_ = 0;
                        map<uint64_t, vector<function<void(pd::result, const T&)>>> dbc_call_funcs_;
                };

        }
}
