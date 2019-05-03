#pragma once

#include <string>
#include <set>
#include <functional>
#include <mutex>

#define invalid_atom 0

using namespace std;

namespace nora {

        static_assert(sizeof(const char *) == sizeof(size_t), "not a 64bit machine?");

        using atom = size_t;
        
        static string atom_to_string(const atom& atm) {
                return string(reinterpret_cast<const char *>(atm));
        }
                
        extern set<string> string_db;
        extern mutex string_db_lock;

        template <typename... Args>
        static atom atom_emplace(Args&&... args) {
                static_assert(is_constructible<string, Args...>::value,
                              "parameter cannot be used to construct a string");
                lock_guard<mutex> lock(string_db_lock);
                return reinterpret_cast<atom>(string_db.emplace(forward<Args>(args)...).first->c_str());
        }

}

