#pragma once

#include "dbcache_logic.hpp"
#include "utils/assert.hpp"
#include "utils/instance_counter.hpp"
#include "dbcache_logic.hpp"
#include <boost/any.hpp>
#include <vector>

using namespace std;

namespace nora {
        namespace dbcache {

                template <typename T>
                class dbc_object : private instance_countee {
                public:
                        dbc_object(instance_class_type ic_type) : instance_countee(ic_type) {
                        }
                        const T& data() const {
                                return data_;
                        }
                        void set(const T& data) {
                                data_ = data;
                        }
                        void parse(const string& data_str) {
                                data_.ParseFromString(data_str);
                        }
                        void update(const T& data) {
                                dirty_ += 1;
                                dbcache_update(data_, data);
                        }
                        void update(const function<void(T&)>& update_func) {
                                dirty_ += 1;
                                ASSERT(update_func);
                                update_func(data_);
                        }
                        int dirty() const {
                                return dirty_;
                        }
                        void reset_dirty() {
                                dirty_ = 0;
                        }
                        uint64_t gid() const {
                                return data_.gid();
                        }
                private:
                        T data_;
                        int dirty_ = 0;
                };

        }
}
