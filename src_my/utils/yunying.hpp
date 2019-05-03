#pragma once

#include "proto/data_event.pb.h"
#include "proto/data_yunying.pb.h"
#include "proto/config_shop.pb.h"
#include <string>
#include <map>

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {

        string yunying_calc_sign(const map<string, string>& params, const string& app_key);

        struct CURL;
        string url_encode(const string& str);
        string url_decode(const string& str);

        bool yunying_item_to_dec_event(const string& item, pd::event& event);
        bool yunying_item_to_add_event(const string& item, pd::event& event);
        string yunying_serialize_item(uint32_t pttid);
        string yunying_serialize_equipment(uint32_t pttid);
        string yunying_serialize_spine(pd::spine_part part, uint32_t pttid);
        string yunying_serialize_huanzhuang(pd::huanzhuang_part part, uint32_t pttid);
        uint32_t yunying_serialize_resource(pd::resource_type type);
        pd::money_list_array yunying_money_list();
        template <typename T>
        void int_to_enum(T& type, uint32_t value) {
                type = static_cast<T>(value);
        }
}
