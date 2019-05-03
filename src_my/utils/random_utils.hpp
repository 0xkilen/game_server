#pragma once

#include "log.hpp"

#include <iostream>
#include <list>
#include <mutex>

using namespace std;

namespace nora {

        class random_seeds {
        public:
                static random_seeds& instance() {
                        static random_seeds inst;
                        return inst;
                }

                void clear();
                void push(uint32_t new_seed);
                uint32_t get();

        private:
                mutable mutex seeds_lock_;
                list<uint32_t> seeds_;
        };

        inline void random_seeds::push(uint32_t new_seed) {
                lock_guard<mutex> lock(seeds_lock_);
                seeds_.push_back(new_seed);
        }

        inline uint32_t random_seeds::get() {
                lock_guard<mutex> lock(seeds_lock_);
                uint32_t battle_seed;
                if (seeds_.empty()) {
                        battle_seed = time(nullptr);
                } else {
                        battle_seed = seeds_.front();
                        seeds_.pop_front();
                }
                DLOG << "random_seeds seed: " << battle_seed;
                return battle_seed;
        }

        inline void random_seeds::clear() {
                lock_guard<mutex> lock(seeds_lock_);
                seeds_.clear();
        }

}
