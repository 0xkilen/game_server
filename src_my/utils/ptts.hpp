#pragma once

#include "log.hpp"
#include "exception.hpp"
#include "assert.hpp"
#include "string_utils.hpp"
#include "json2pb.hpp"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <functional>
#include <list>
#include <vector>
#include <mutex>
#include <boost/filesystem.hpp>

using namespace std;
namespace gp = google::protobuf;
namespace bfs = boost::filesystem;

#define CONFIG_DLOG __DLOG << setw(20) << "[CONFIG] "
#define CONFIG_ILOG __ILOG << setw(20) << "[CONFIG] "
#define CONFIG_ELOG __ELOG << setw(20) << "[CONFIG] "

namespace nora {

        extern mutex ptts_lock;
        extern string config_path;

        template <typename T>
        class ptts {
        public:
                void cout_csv_template() const;
                vector<string> find_csv_files(const string& type);
                template <typename F>
                void load_json_file(const string& file);
                void load_csv_file(const string& file);
                void load_csv_files(const string& type);
                template <typename F>
                void cout_json_file(const string& csv_file, F *f);
                template <typename F>
                void cout_bin_file(const string& type, F *f);
                template <typename F>
                void write_all_file(const string& type, F *f);
                void check();
                void modify();
                void verify() const;
                void print() const;
                void enrich();
                const T& get_ptt(uint32_t key) const;
                T& get_ptt(uint32_t key);
                T get_ptt_copy(uint32_t key) const;
                const map<uint32_t, T>& get_ptt_all() const;
                map<uint32_t, T> get_ptt_all_copy() const;
                bool has_ptt(uint32_t key);

                function<void(const T&)> check_func_;
                function<void(T&)> modify_func_;
                function<void(const T&)> verify_func_;
                function<void(map<uint32_t, T>&)> enrich_func_;
        private:
                void write_message(const gp::Descriptor *desc, ostringstream& oss, string prefix) const;
                list<string> load_header_line(ifstream& ifs);
                void load_fields(gp::Message& msg, istringstream& iss, string header, list<string>& headers);

                map<uint32_t, T> ptts_;
        };

        inline string append_header(const string& header, const string& str) {
                string ret = header;
                if (!ret.empty()) {
                        ret += ".";
                }
                ret += str;
                return ret;
        }

        inline bool invisible_header(const string& header) {
                return header[0] == '_';
        }

        template <typename T>
        void ptts<T>::write_message(const gp::Descriptor *desc, ostringstream& oss, string prefix) const {
                for (int i = 0; i < desc->field_count(); ++i) {
                        auto *field = desc->field(i);
                        if (invisible_header(field->name())) {
                                continue;
                        }
                        int count = field->is_repeated() ? 1 : 1;
                        for (int i = 0; i < count; ++i) {
                                if (field->type() == gp::FieldDescriptor::TYPE_MESSAGE) {
                                        write_message(field->message_type(), oss, prefix.empty() ? field->name() : prefix + "." + field->name());
                                } else {
                                        if (!prefix.empty()) {
                                                oss << prefix << ".";
                                        }
                                        oss << field->name();
                                        oss << ",";
                                }
                        }

                }
        }

        template <typename T>
        void ptts<T>::cout_csv_template() const {
                auto *desc = T::descriptor();
                auto *first_field = desc->field(0);
                if (first_field->type() != gp::FieldDescriptor::TYPE_UINT32 && first_field->type() != gp::FieldDescriptor::TYPE_ENUM) {
                        CONFIG_ELOG << desc->name() << "'s first member is neither uint32 nor enum";
                        return;
                }

                ostringstream oss;
                write_message(desc, oss, "");

                oss.seekp(-1, std::ios_base::end);
                oss << "\n";
                cout << oss.str();
        }

        template <typename T>
        void ptts<T>::load_fields(gp::Message& msg, istringstream& iss, string header, list<string>& headers) {
                auto *desc = msg.GetDescriptor();
                auto *refl = msg.GetReflection();

                string grid;
                for (auto i = 0; i < desc->field_count(); ++i) {
                        const auto *field = desc->field(i);
                        if (invisible_header(field->name())) {
                                continue;
                        }
                        auto cur_header = append_header(header, field->name());
                        if (headers.empty()) {
                                CONFIG_ELOG << desc->name() << " run out of header when processing " << cur_header;
                                return;
                        }
                        if (field->cpp_type() == gp::FieldDescriptor::CPPTYPE_MESSAGE) {
                                if (field->is_repeated()) {
                                        // CONFIG_DLOG << "process repeated message: " << field->name();
                                        const auto *msg_desc = field->message_type();
                                        // CONFIG_DLOG << "first left headers: " << headers.front();
                                        auto next_header = append_header(cur_header, msg_desc->field(0)->name());
                                        // CONFIG_DLOG << "next header: " << next_header;
                                        while (!headers.empty() && headers.front().find(next_header) == 0) {
                                                // CONFIG_DLOG << "found one";
                                                auto *sub_msg = refl->AddMessage(&msg, field);
                                                load_fields(*sub_msg, iss, cur_header, headers);
                                                if (sub_msg->ByteSize() <= 0) {
                                                        refl->RemoveLast(&msg, field);
                                                }
                                        }
                                } else {
                                        auto *sub_msg = refl->MutableMessage(&msg, field);
                                        load_fields(*sub_msg, iss, cur_header, headers);
                                        if (sub_msg->ByteSize() <= 0) {
                                                refl->ClearField(&msg, field);
                                        }
                                }
                        } else if (field->is_repeated()) {
                                while (!headers.empty() && headers.front() == cur_header) {
                                        getline(iss, grid);
                                        headers.pop_front();
                                        if (string(grid.data()).empty()) continue;

                                        switch (field->cpp_type()) {
                                        case gp::FieldDescriptor::CPPTYPE_BOOL:
                                                try {
                                                        auto value = false;
                                                        string str = grid.data();
                                                        transform(str.begin(), str.end(), str.begin(), ::toupper);
                                                        if (grid.data() == string("TRUE")) {
                                                                value = true;
                                                        }
                                                        refl->AddBool(&msg, field, value);
                                                } catch (const invalid_argument& e) {
                                                        CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                        return;
                                                }
                                                break;
                                        case gp::FieldDescriptor::CPPTYPE_UINT32:
                                                try {
                                                        if (grid.empty()) {
                                                                refl->AddUInt32(&msg, field, 0u);
                                                        } else {
                                                                refl->AddUInt32(&msg, field, stoul(grid.data()));
                                                        }
                                                } catch (const invalid_argument& e) {
                                                        CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                        return;
                                                }
                                                break;
                                        case gp::FieldDescriptor::CPPTYPE_INT32:
                                                try {
                                                        if (grid.empty()) {
                                                                refl->AddInt32(&msg, field, 0);
                                                        } else {
                                                                refl->AddInt32(&msg, field, stoi(grid.data()));
                                                        }
                                                } catch (const invalid_argument& e) {
                                                        CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                        return;
                                                }
                                                break;
                                        case gp::FieldDescriptor::CPPTYPE_INT64:
                                                try {
                                                        if (grid.empty()) {
                                                                refl->AddInt64(&msg, field, 0);
                                                        } else {
                                                                refl->AddInt64(&msg, field, stoi(grid.data()));
                                                        }
                                                } catch (const invalid_argument& e) {
                                                        CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                        return;
                                                }
                                                break;
                                        case gp::FieldDescriptor::CPPTYPE_UINT64:
                                                try {
                                                        if (grid.empty()) {
                                                                refl->AddUInt64(&msg, field, 0ul);
                                                        } else {
                                                                refl->AddUInt64(&msg, field, stoull(grid.data()));
                                                        }
                                                } catch (const invalid_argument& e) {
                                                        CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                        return;
                                                }
                                                break;
                                        case gp::FieldDescriptor::CPPTYPE_DOUBLE:
                                                try {
                                                        if (grid.empty()) {
                                                                refl->AddDouble(&msg, field, 0.);
                                                        } else {
                                                                refl->AddDouble(&msg, field, stod(grid.data()));
                                                        }
                                                } catch (const invalid_argument& e) {
                                                        CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                        return;
                                                }
                                                break;
                                        case gp::FieldDescriptor::CPPTYPE_STRING:
                                                refl->AddString(&msg, field, grid.data());
                                                break;
                                        case gp::FieldDescriptor::CPPTYPE_ENUM: {
                                                const auto *ed = field->enum_type();
                                                const auto *evd = ed->FindValueByName(grid.data());
                                                if (!evd) {
                                                        CONFIG_ELOG << field->name() << " has invalid enum value " << grid.data();
                                                        return;
                                                }
                                                refl->AddEnum(&msg, field, evd);
                                        }
                                                break;
                                        default:
                                                CONFIG_ELOG << field->name() << " has invalid type " << field->cpp_type_name();
                                                return;
                                        }
                                }
                        } else {
                                getline(iss, grid);
                                headers.pop_front();
                                if (string(grid.data()).empty()) continue;
                                switch (field->cpp_type()) {
                                case gp::FieldDescriptor::CPPTYPE_BOOL:
                                        try {
                                                auto value = false;
                                                string str = grid.data();
                                                transform(str.begin(), str.end(), str.begin(), ::toupper);
                                                if (str == string("TRUE")) {
                                                        value = true;
                                                }
                                                refl->SetBool(&msg, field, value);
                                        } catch (const invalid_argument& e) {
                                                CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                return;
                                        }
                                        break;
                                case gp::FieldDescriptor::CPPTYPE_UINT32:
                                        try {
                                                refl->SetUInt32(&msg, field, stoul(grid.data()));
                                        } catch (const invalid_argument& e) {
                                                CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                return;
                                        }
                                        break;
                                case gp::FieldDescriptor::CPPTYPE_INT32:
                                        try {
                                                refl->SetInt32(&msg, field, stoi(grid.data()));
                                        } catch (const invalid_argument& e) {
                                                CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                return;
                                        }
                                        break;
                                case gp::FieldDescriptor::CPPTYPE_INT64:
                                        try {
                                                refl->SetInt64(&msg, field, stol(grid.data()));
                                        } catch (const invalid_argument& e) {
                                                CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                return;
                                        }
                                        break;
                                case gp::FieldDescriptor::CPPTYPE_UINT64:
                                        try {
                                                refl->SetUInt64(&msg, field, stoull(grid.data()));
                                        } catch (const invalid_argument& e) {
                                                CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                return;
                                        }
                                        break;
                                case gp::FieldDescriptor::CPPTYPE_DOUBLE:
                                        try {
                                                refl->SetDouble(&msg, field, stod(grid.data()));
                                        } catch (const invalid_argument& e) {
                                                CONFIG_ELOG << field->name() << " has invalid value " << grid.data();
                                                return;
                                        }
                                        break;
                                case gp::FieldDescriptor::CPPTYPE_STRING:
                                        refl->SetString(&msg, field, grid.data());
                                        break;
                                case gp::FieldDescriptor::CPPTYPE_ENUM: {
                                        const auto *ed = field->enum_type();
                                        const auto *evd = ed->FindValueByName(grid.data());
                                        if (!evd) {
                                                CONFIG_ELOG << field->name() << " has invalid enum value " << grid.data();
                                                return;
                                        }
                                        refl->SetEnum(&msg, field, evd);
                                        break;
                                }
                                default:
                                        CONFIG_ELOG << field->name() << " has invalid type " << field->cpp_type_name();
                                        return;
                                }
                        }
                }
        }

        template <typename T>
        list<string> ptts<T>::load_header_line(ifstream& ifs) {
                list<string> ret;

                string line;
                getline(ifs, line);
                replace(line.begin(), line.end(), ',', '\n');
                replace(line.begin(), line.end(), '\r', '\n');

                istringstream iss(line);
                string grid;
                while (getline(iss, grid)) {
                        ret.push_back(grid);
                }

                return ret;
        }

        template <typename T>
        template <typename F>
        void ptts<T>::load_json_file(const string& file) {
                lock_guard<mutex> lock(ptts_lock);
                ptts_.clear();
                if (!ptts_.empty()) {
                        return;
                }

                ifstream ifs(file);
                std::string str;

                ifs.seekg(0, std::ios::end);
                str.reserve(ifs.tellg());
                ifs.seekg(0, std::ios::beg);

                str.assign((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());

                F f;
                json2pb(f, str.data(), str.size());

                auto *desc = T::descriptor();
                auto *first_field = desc->field(0);
                for (const auto& i : f.entry()) {
                        const auto* reflection = i.GetReflection();
                        ptts_[reflection->GetUInt32(i, first_field)] = i;
                }
        }

        template <typename T>
        vector<string> ptts<T>::find_csv_files(const string& type) {
                vector<string> ret;

                ostringstream oss;
                oss << config_path << "/" << type << "_table.csv";
                if (bfs::exists(bfs::path(oss.str()))) {
                        ret.push_back(oss.str());
                } else {
                        int i = 1;
                        do {
                                oss.str("");
                                oss << config_path << "/" << type << "_table_" << i << ".csv";
                                if (!bfs::exists(bfs::path(oss.str()))) {
                                        break;
                                }

                                ret.push_back(oss.str());
                                i += 1;
                        } while (true);
                }

                return ret;
        }

        template <typename T>
        void ptts<T>::load_csv_files(const string& type) {
                lock_guard<mutex> lock(ptts_lock);
                ptts_.clear();
                auto files = find_csv_files(type);
                for (const auto& i : files) {
                        load_csv_file(i);
                }
        }

        template <typename T>
        void ptts<T>::load_csv_file(const string& file) {
                ifstream ifs(file);
                auto *desc = T::descriptor();

                auto *first_field = desc->field(0);
                if (first_field->type() != gp::FieldDescriptor::TYPE_UINT32 && first_field->type() != gp::FieldDescriptor::TYPE_ENUM) {
                        CONFIG_ELOG << desc->name() << "'s first member is not uint32";
                        return;
                }

                auto headers = load_header_line(ifs);

                // load all data lines
                string line;
                while (getline(ifs, line)) {
                        replace(line.begin(), line.end(), ',', '\n');
                        replace(line.begin(), line.end(), '\r', '\n');
                        istringstream iss(line.data());
                        if (iss.str().empty()) continue;

                        T t;
                        auto headers_copy = headers;
                        load_fields(t, iss, "", headers_copy);

                        uint32_t key = 0;
                        auto *refl = t.GetReflection();
                        if (first_field->type() == gp::FieldDescriptor::TYPE_UINT32) {
                                key = refl->GetUInt32(t, desc->field(0));
                        } else {
                                ASSERT(first_field->type() == gp::FieldDescriptor::TYPE_ENUM);
                                ASSERT(desc->field(0)->number() >= 0);
                                key = static_cast<uint32_t>(refl->GetEnum(t, desc->field(0))->number());
                        }

                        if (ptts_.count(key) != 0) {
                                CONFIG_ELOG << desc->name() << " has duplicate " << first_field->name() << ": " << key;
                                return;
                        }
                        ptts_[key] = t;
                }
        }

        template <typename T>
        template <typename F>
        void ptts<T>::cout_json_file(const string& csv_file, F *f) {
                load_csv_file(csv_file);
                if (ptts_.empty()) {
                        return;
                }

                for (auto& i : ptts_) {
                        auto *entry = f->add_entry();
                        *entry = i.second;
                }

                auto json_str = pb2json(*f);
                cout << json_str;
        }

        template <typename T>
        template <typename F>
        void ptts<T>::cout_bin_file(const string& csv_file, F *f) {
                load_csv_file(csv_file);
                if (ptts_.empty()) {
                        return;
                }
                vector<int> keys;
                for (auto& i : ptts_) {
                        keys.push_back(i.first);
                }
                sort(keys.begin(), keys.end(),
                     [] (auto a, auto b) {
                             return a < b;
                     });
                for_each(keys.begin(),keys.end(),[=](int key){
                                auto *entry = f->add_entry();
                                *entry = ptts_[key];
                        });
                string bin_str;
                f->SerializeToString(&bin_str);
                cout << bin_str;
        }

        template <typename T>
        template <typename F>
        void ptts<T>::write_all_file(const string& type, F *f) {
                load_csv_files(type);
                if (enrich_func_) {
                        enrich_func_(ptts_);
                }
                auto ptts = ptts_;

                auto files = find_csv_files(type);
                for (const auto& i : files) {
                        ptts_.clear();
                        f->clear_entry();
                        load_csv_file(i);

                        if (ptts_.empty()) {
                                return;
                        }
                        vector<int> keys;
                        for (auto& j : ptts_) {
                                keys.push_back(j.first);
                        }
                        sort(keys.begin(), keys.end(),
                             [] (auto a, auto b) {
                                     return a < b;
                             });
                        for_each(keys.begin(), keys.end(),
                                 [this, f, &ptts] (auto key){
                                         *f->add_entry() = ptts[key];
                                 });

                        string file = i;
                        ASSERT(replace_string(file, ".csv", ".config"));
                        ASSERT(replace_string(file, "_table", ""));
                        ofstream bin_file(file, ios_base::binary|ios_base::out|ios_base::trunc);
                        f->SerializeToOstream(&bin_file);

                        file = i;
                        ASSERT(replace_string(file, ".csv", ".json"));
                        ASSERT(replace_string(file, "_table", ""));
                        ofstream json_file(file, ios_base::binary|ios_base::out|ios_base::trunc);
                        json_file << pb2json(*f);
                }
        }

        template <typename T>
        void ptts<T>::check() {
                if (check_func_) {
                        for (auto& i : ptts_) {
                                if (i.first >= 1000000000) {
                                        CONFIG_ELOG << " config id used more than 9 digits " << i.first;
                                }
                                check_func_(i.second);
                                if (!i.second.IsInitialized()) {
                                        CONFIG_ELOG << i.first << i.second.InitializationErrorString();
                                }
                        }
                }
        }

        template <typename T>
        void ptts<T>::modify() {
                if (modify_func_) {
                        for (auto& i : ptts_) {
                                modify_func_(i.second);
                        }
                }
        }

        template <typename T>
        void ptts<T>::verify() const {
                if (verify_func_) {
                        for (const auto& i : ptts_) {
                                verify_func_(i.second);
                                if (!i.second.IsInitialized()) {
                                        CONFIG_ELOG << i.first << " not initialized: " << i.second.InitializationErrorString();
                                        continue;
                                }
                        };
                }
        }

        template <typename T>
        void ptts<T>::print() const {
                for (const auto& i : ptts_) {
                        CONFIG_ILOG << "\n" << i.second.DebugString();
                }
        }

        template <typename T>
        void ptts<T>::enrich() {
                if (enrich_func_) {
                        enrich_func_(ptts_);
                }
        }

        template <typename T>
        const T& ptts<T>::get_ptt(uint32_t key) const {
                lock_guard<mutex> lock(ptts_lock);
                ASSERT(ptts_.count(key) > 0);
                return ptts_.at(key);
        }

        template <typename T>
        T& ptts<T>::get_ptt(uint32_t key) {
                lock_guard<mutex> lock(ptts_lock);
                ASSERT(ptts_.count(key) > 0);
                return ptts_.at(key);
        }

        template <typename T>
        T ptts<T>::get_ptt_copy(uint32_t key) const {
                lock_guard<mutex> lock(ptts_lock);
                ASSERT(ptts_.count(key) > 0);
                return ptts_.at(key);
        }

        template <typename T>
        const map<uint32_t, T>& ptts<T>::get_ptt_all() const {
                lock_guard<mutex> lock(ptts_lock);
                return ptts_;
        }

        template <typename T>
        map<uint32_t, T> ptts<T>::get_ptt_all_copy() const {
                lock_guard<mutex> lock(ptts_lock);
                return ptts_;
        }

        template <typename T>
        bool ptts<T>::has_ptt(uint32_t key) {
                lock_guard<mutex> lock(ptts_lock);
                return ptts_.count(key) > 0;
        }

}
