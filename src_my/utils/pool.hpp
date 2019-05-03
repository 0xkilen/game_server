#pragma once

#include <boost/core/noncopyable.hpp>
#include <queue>
#include <memory>

using namespace std;

namespace nora {

        template<typename T>
        class pool : private boost::noncopyable {
        public:
                static pool<T>& instance() {
                        static pool<T> inst;
                        return inst;
                }
                template <typename... Args>
                void init(int count, Args &&... arg) {
                        lock_guard<mutex> lock(lock_);
                        for (auto i = 0; i < count; ++i) {
                                pool_.push(new T(forward<Args>(arg)...));
                        }
                }
                template <typename... Args>
                shared_ptr<T> get(Args &&... arg) {
                        lock_guard<mutex> lock(lock_);
                        if (!pool_.empty()) {
                                auto *obj = pool_.front();
                                pool_.pop();
                                obj->init(forward<Args>(arg)...);
                                return shared_ptr<T>(
                                        obj,
                                        [this] (T *delete_obj) {
                                                delete_obj->destroy();
                                                lock_guard<mutex> lock(lock_);
                                                pool_.push(delete_obj);
                                        });
                        } else {
                                auto *obj = new T();
                                obj->init(forward<Args>(arg)...);
                                return shared_ptr<T>(
                                        obj,
                                        [this] (T *delete_obj) {
                                                delete_obj->destroy();
                                                lock_guard<mutex> lock(lock_);
                                                pool_.push(delete_obj);
                                        });
                        }
                }
                void clean_up() {
                        while (!pool_.empty()) {
                                auto *obj = pool_.front();
                                pool_.pop();
                                delete obj;
                        }
                }
        private:
                mutable mutex lock_;
                queue<T *> pool_;
        };

}
