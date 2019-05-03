#pragma once

#include <functional>
#include <list>
#include <mutex>

using namespace std;

namespace nora {

        template <typename Ret, typename... Args>
        class functions {
        public:
                void operator()(Args... args);
                typename list<function<Ret(Args...)>>::iterator add(const function<Ret(Args...)>& func);
                void remove(typename list<function<Ret(Args...)>>::iterator func_id);
                void clear();
        private:
                mutable mutex funcs_lock_;
                list<function<Ret(Args...)>> funcs_;
        };

        template <typename Ret, typename... Args>
        inline typename list<function<Ret(Args...)>>::iterator functions<Ret, Args...>::add(const function<Ret(Args...)>& func) {
                lock_guard<mutex> lock(funcs_lock_);
                funcs_.push_front(func);
                return funcs_.begin();
        }

        template <typename Ret, typename... Args>
        inline void functions<Ret, Args...>::operator()(Args... args) {
                lock_guard<mutex> lock(funcs_lock_);
                for (auto& f : funcs_) {
                        f(args...);
                }
        }

        template <typename Ret, typename... Args>
        inline void functions<Ret, Args...>::clear() {
                lock_guard<mutex> lock(funcs_lock_);
                funcs_.clear();
        }

        template <typename Ret, typename... Args>
        inline void functions<Ret, Args...>::remove(typename list<function<Ret(Args...)>>::iterator func_id) {
                lock_guard<mutex> lock(funcs_lock_);
                funcs_.erase(func_id);
        }

}

