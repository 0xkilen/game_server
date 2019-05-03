#pragma once

#include "config/behavior_tree_ptts.hpp"
#include "proto/data_base.pb.h"
#include "proto/data_behavior_tree.pb.h"
#include "scene/log.hpp"
#include "utils/service_thread.hpp"
#include <memory>
#include <queue>
#include <chrono>

using namespace std;
using namespace std::chrono;
namespace pc = proto::config;

#define SCENE_BT_TLOG __TLOG << setw(20) << "[SCENE_BT] "
#define SCENE_BT_DLOG __DLOG << setw(20) << "[SCENE_BT] "
#define SCENE_BT_ILOG __ILOG << setw(20) << "[SCENE_BT] "
#define SCENE_BT_ELOG __ELOG << setw(20) << "[SCENE_BT] "

namespace nora {
        namespace scene {

                template <typename T>
                class behavior_tree : public enable_shared_from_this<behavior_tree<T>> {
                public:
                        behavior_tree(T *owner,
                                      const shared_ptr<service_thread>& st,
                                      uint32_t default_root) :
                                owner_(owner),
                                st_(st),
                                default_root_(default_root),
                                cur_root_(default_root) {
                                ASSERT(owner_);
                                next_frequency_ = system_clock::duration::zero();
                        }
                        void set_rand_func(const function<int()>& func) {
                                rand_ = func;
                        }
                        void run() {
                                ASSERT(st_ && st_->check_in_thread());
                                if (timer_) {
                                        return;
                                }
                                if (stop_) {
                                        stop_ = false;
                                        return;
                                }

                                timer_ = ADD_TIMER(
                                        st_,
                                        ([this, self = this->shared_from_this()] (auto canceled, const auto& timer) {
                                                if (timer_ == timer) {
                                                        timer_.reset();
                                                }
                                                if (!canceled && !stop_) {
                                                        this->process_root();
                                                        this->run();
                                                }
                                        }),
                                        next_frequency_ == system_clock::duration::zero() ?
                                        default_frequency_ : next_frequency_);
                                next_frequency_ = system_clock::duration::zero();
                        }
                        bool running() const {
                                return !stop_;
                        }
                        void rebuild_root_node(uint32_t behavior_tree_root, uint32_t children_node) {
                                cur_root_ = behavior_tree_root;
                                auto& ptt = PTTS_GET(behavior_tree, cur_root_);
                                ptt.clear_children();
                                ptt.add_children(children_node);
                                pd::behavior_tree_node ret;
                                ret.set_pttid(cur_root_);
                                for (auto i : ptt.children()) {
                                        *ret.add_children() = build_node(i);
                                }
                                roots_[cur_root_] = ret;
                        }
                        void process_root() {
                                if (!root_queue_.empty()) {
                                        cur_root_ = root_queue_.front();
                                        root_queue_.pop();
                                }

                                ASSERT(cur_root_ > 0);
                                if (roots_.count(cur_root_) == 0) {
                                        roots_[cur_root_] = build_node(cur_root_);
                                }

                                auto& root = roots_.at(cur_root_);
                                try {
                                        auto result = process(root);
                                        if (result == pd::OK || result == pd::FAILED) {
                                                reset(root);
                                                if (!root_queue_.empty()) {
                                                        cur_root_ = root_queue_.front();
                                                        root_queue_.pop();
                                                } else {
                                                        if (only_queue_) {
                                                                stop();
                                                        }
                                                        cur_root_ = default_root_;
                                                }
                                        }
                                } catch (...) {
                                        SCENE_BT_TLOG << "process exception";
                                        reset(root);
                                }
                        }

                        void set_only_queue(bool only_queue) {
                                only_queue_ = only_queue;
                        }
                        void push_root(uint32_t root) {
                                root_queue_.push(root);
                        }
                        pd::result process(pd::behavior_tree_node& node) {
                                const auto& ptt = PTTS_GET(behavior_tree, node.pttid());

                                auto result = pd::NONE;
                                if (node.cache().has_status() && node.cache().status() != pd::RUNNING) {
                                        SCENE_BT_TLOG << "bt node use cache status";
                                        result = node.cache().status();
                                } else {
                                        switch (ptt.type()) {
                                        case pc::BNT_ACTION: {
                                                cur_action_node_ = &node;
                                                result = owner_->behavior_tree_action(node);
                                                if (result == pd::OK || result == pd::FAILED) {
                                                        cur_action_node_ = nullptr;
                                                }
                                                break;
                                        }
                                        case pc::BNT_CONDITION:
                                                result = owner_->behavior_tree_condition(node);
                                                break;
                                        case pc::BNT_FALLBACK:
                                                ASSERT(node.children_size() > 0);
                                                SCENE_BT_TLOG << "BNT_FALLBACK node cache child_idx is " << node.cache().child_idx();
                                                for (auto i = node.cache().child_idx(); i < node.children_size(); ++i) {
                                                        auto& child = *node.mutable_children(i);
                                                        result = process(child);
                                                        if (result == pd::FAILED) {
                                                                node.mutable_cache()->set_child_idx(i + 1);
                                                                continue;
                                                        } else {
                                                                break;
                                                        }
                                                }
                                                break;
                                        case pc::BNT_RANDOM_FALLBACK: {
                                                ASSERT(node.children_size() > 0);
                                                if (node.cache().children_idx_size() > 0) {
                                                        auto child_idx = node.cache().children_idx(node.cache().children_idx_size() - 1);
                                                        auto& child = *node.mutable_children(child_idx);
                                                        result = process(child);
                                                }

                                                if (result == pd::NONE || result == pd::FAILED) {
                                                        set<int> not_tried_children_idxs;
                                                        for (auto i = 0; i < node.children_size(); ++i) {
                                                                not_tried_children_idxs.insert(i);
                                                        }
                                                        for (auto i : node.cache().children_idx()) {
                                                                not_tried_children_idxs.erase(i);
                                                        }

                                                        while (!not_tried_children_idxs.empty()) {
                                                                auto iter = not_tried_children_idxs.begin();
                                                                auto randnumber = rand_ ? rand_() : rand();
                                                                SCENE_BT_TLOG  << "BNT_RANDOM_FALLBACK rand number is " << randnumber << " index is " << randnumber % not_tried_children_idxs.size();
                                                                auto dis = randnumber % not_tried_children_idxs.size();
                                                                advance(iter, dis);
                                                                auto& child = *node.mutable_children(*iter);
                                                                result = process(child);
                                                                node.mutable_cache()->add_children_idx(*iter);
                                                                not_tried_children_idxs.erase(*iter);
                                                                if (result == pd::OK || result == pd::RUNNING) {
                                                                        break;
                                                                } else {
                                                                        continue;
                                                                }
                                                        }
                                                }

                                                if (result == pd::OK || result == pd::FAILED) {
                                                        node.mutable_cache()->clear_children_idx();
                                                }
                                                break;
                                        }
                                        case pc::BNT_SEQUENCE:
                                                ASSERT(node.children_size() > 0);
                                                SCENE_BT_TLOG << "BNT_SEQUENCE node cache child_idx is " << node.cache().child_idx();
                                                for (auto i = node.cache().child_idx(); i < node.children_size(); ++i) {
                                                        auto& child = *node.mutable_children(i);
                                                        result = process(child);
                                                        if (result == pd::OK) {
                                                                node.mutable_cache()->set_child_idx(i + 1);
                                                                continue;
                                                        } else {
                                                                break;
                                                        }
                                                }
                                                break;
                                        case pc::BNT_PARALLEL: {
                                                ASSERT(node.children_size() > 0);
                                                int ok_count = 0;
                                                int failed_count = 0;
                                                for (auto& i : *node.mutable_children()) {
                                                        result = process(i);
                                                        if (result == pd::OK) {
                                                                ok_count += 1;
                                                                break;
                                                        } else if (result == pd::FAILED) {
                                                                failed_count += 1;
                                                                break;
                                                        } else {
                                                                break;
                                                        }
                                                }

                                                if (ok_count >= ptt.parallel().threshold()) {
                                                        result = pd::OK;
                                                } else if (failed_count > node.children_size() - ptt.parallel().threshold()) {
                                                        result = pd::FAILED;
                                                } else {
                                                        result = pd::RUNNING;
                                                }
                                                break;
                                        }
                                        }

                                        ASSERT(result != pd::NONE);
                                        node.mutable_cache()->set_status(result);
                                        if (result == pd::RUNNING) {
                                                auto running_times = node.cache().running_times();
                                                running_times += 1;
                                                node.mutable_cache()->set_running_times(running_times);
                                                if (ptt.has_desc() && running_times > 32) {
                                                        SCENE_BT_ELOG << *owner_ << " possible dead end: " << ptt.desc() << " running times: " << running_times;
                                                }
                                        }
                                }
                                SCENE_BT_DLOG << *owner_ << " process node: " << ptt.desc() << " result: " << pd::result_Name(result);
                                return result;
                        }
                        pd::behavior_tree_node *cur_action_node() {
                                return cur_action_node_;
                        }
                        void stop() {
                                ASSERT(st_->check_in_thread());
                                if (stop_) {
                                        return;
                                }
                                stop_ = true;
                                if (timer_) {
                                        timer_->cancel();
                                        timer_.reset();
                                }
                                for (auto& i : roots_) {
                                        reset(i.second);
                                }
                        }
                        void set_next_frequency(const system_clock::duration& du) {
                                next_frequency_ = du;
                        }
                private:
                        pd::behavior_tree_node build_node(uint32_t pttid) {
                                pd::behavior_tree_node ret;
                                ret.set_pttid(pttid);
                                const auto& ptt = PTTS_GET(behavior_tree, pttid);
                                for (auto i : ptt.children()) {
                                        *ret.add_children() = build_node(i);
                                }
                                return ret;
                        }

                        void reset(pd::behavior_tree_node& node) {
                                node.clear_cache();
                                for (auto& i : *node.mutable_children()) {
                                        reset(i);
                                }
                                cur_action_node_ = nullptr;
                        }

                        T *owner_;
                        shared_ptr<service_thread> st_;
                        map<uint32_t, pd::behavior_tree_node> roots_;
                        uint32_t default_root_ = 0;
                        uint32_t cur_root_ = 0;
                        queue<uint32_t> root_queue_;
                        pd::behavior_tree_node *cur_action_node_ = nullptr;
                        shared_ptr<timer_type> timer_;
                        bool stop_ = false;
                        system_clock::duration next_frequency_;
                        system_clock::duration default_frequency_ = 3s;
                        function<int()> rand_;
                        bool only_queue_ = false;
                };

        }
}
