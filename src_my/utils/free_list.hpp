#pragma once

#include "assert.hpp"
#include <map>
#include <list>

using namespace std;

namespace nora {

        class free_list {
        public:
                void push(uint64_t gid);
                uint64_t pop();
                uint64_t peek() const;
                void touch(uint64_t gid);
                bool has(uint64_t gid) const;
                size_t size() const;
                bool empty() const;
                void remove(uint64_t gid);
                void clear();
        private:
                map<uint64_t, list<uint64_t>::iterator> gid2iters_;
                list<uint64_t> list_;
        };

        inline bool free_list::empty() const {
                return list_.empty();
        }

        inline bool free_list::has(uint64_t gid) const {
                return gid2iters_.count(gid) > 0;
        }

        inline void free_list::push(uint64_t gid) {
                ASSERT(gid2iters_.count(gid) == 0);
                list_.push_back(gid);
                gid2iters_[gid] = --list_.end();
        }

        inline uint64_t free_list::pop() {
                ASSERT(!empty());
                uint64_t ret = list_.front();
                list_.pop_front();
                gid2iters_.erase(ret);
                return ret;
        }

        inline uint64_t free_list::peek() const {
                ASSERT(!empty());
                return list_.front();
        }

        inline void free_list::touch(uint64_t gid) {
                if (has(gid)) {
                        remove(gid);
                }
                push(gid);
        }

        inline size_t free_list::size() const {
                return list_.size();
        }

        inline void free_list::remove(uint64_t gid) {
                ASSERT(gid2iters_.count(gid) > 0);
                list_.erase(gid2iters_.at(gid));
                gid2iters_.erase(gid);
        }

        inline void free_list::clear() {
                list_.clear();
                gid2iters_.clear();
        }


}
