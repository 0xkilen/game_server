#include "yunying.hpp"
#include "utils/log.hpp"
#include "utils/crypto.hpp"
#include "utils/assert.hpp"
#include "config/item_ptts.hpp"
#include "config/equipment_ptts.hpp"
#include "config/spine_ptts.hpp"
#include "config/resource_ptts.hpp"
#include "config/huanzhuang_ptts.hpp"
#include "config/actor_ptts.hpp"
#include "config/utils.hpp"
#include <openssl/md5.h>
#include <curl/curl.h>
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>

using namespace std;

namespace nora {

        namespace {

                void set_money_list(auto& mrla, uint32_t id, const auto& ptts) {
                        for (const auto& i : ptts) {
                                auto *money_list = mrla.add_money_list();
                                money_list->set_id(id + i.second.id());
                                money_list->set_name(i.second.name());
                                money_list->set_type("1");
                        }
                }

        }

        string yunying_calc_sign(const map<string, string>& params, const string& app_key) {
                ostringstream oss;
                for (const auto& i : params) {
                        if (i.first == "sign") {
                                continue;
                        }
                        oss << i.first;
                        oss << "=";
                        oss << url_encode(i.second);
                        oss << "&";
                }
                oss << app_key;
                auto hash = nora::MD5(oss.str());
                oss.str("");
                oss << hex << setfill('0');
                for (unsigned char i : hash) {
                        oss << setw(2) << static_cast<int>(i);
                }
                return oss.str();
        }

        static ::CURL *curl = curl_easy_init();

        string url_encode(const string& str) {
                char *url_encoded = curl_easy_escape(curl, str.c_str(), str.length());
                string ret(url_encoded);
                curl_free(url_encoded);
                return ret;
        }

        string url_decode(const string& str) {
                char *url_decoded = curl_easy_unescape(curl, str.c_str(), str.length(), nullptr);
                string ret(url_decoded);
                curl_free(url_decoded);
                return ret;
        }

        enum yunying_item_prefix {
                YIP_RESOURCE = 1,
                YIP_ITEM = 2,
                YIP_EQUIPMENT = 3,
                YIP_HONGMO = 4,
                YIP_YANJING = 5,
                YIP_YANSHENGUANG = 6,
                YIP_SAIHONG = 7,
                YIP_MEIMAO = 8,
                YIP_YANZHUANG = 9,
                YIP_CHUNCAI = 11,
                YIP_TIEHUA = 12,
                YIP_LIANXING = 13,
                YIP_YIFU = 14,
                YIP_TOUFA = 15,
                YIP_JIEZHI = 16,
                YIP_EDAI = 17,
                YIP_FAZAN = 18,
                YIP_GUANSHI = 19,
                YIP_TOUJIN = 20,
                YIP_ERHUAN = 21,
                YIP_XIANGLIAN = 22,
                YIP_BEISHI = 23,
                YIP_GUANGHUAN = 24,
                YIP_SHOUCHI = 25,
                YIP_SPINE_DEFORM_FACE = 26,
                YIP_SPINE_DEFORM_HEAD = 27,
                YIP_HUANZHUANG_DEFORM_FACE = 28,
                YIP_HUANZHUANG_DEFORM_HEAD = 29,
                YIP_ZUI = 30,
                YIP_HUZI = 31,
                YIP_LIAN = 32,
                YIP_ACTOR_SKIN = 33,
                YIP_COUNT = 34,
                YIP_BASE = 10000000
        };

        bool yunying_item_to_dec_event(const string& item, pd::event& event) {
                uint32_t prefix = stoul(item) / YIP_BASE;
                if (prefix < 1 || prefix > 2 || prefix >= YIP_COUNT) {
                        return false;
                }
                auto yip = static_cast<yunying_item_prefix>(prefix);
                switch (yip) {
                case YIP_RESOURCE:
                        event.set_type(pd::event::DEC_RESOURCE);
                        break;
                case YIP_ITEM:
                        event.set_type(pd::event::REMOVE_ITEM);
                        break;
                default:
                        return false;
                }

                uint32_t id = stoul(item) % YIP_BASE;
                if (event.type() == pd::event::DEC_RESOURCE) {
                        if (!pd::resource_type_IsValid(id)) {
                                return false;
                        }
                        event.add_arg(pd::resource_type_Name(static_cast<pd::resource_type>(id)));
                } else {
                        event.add_arg(to_string(id));
                }

                return true;
        }

        bool yunying_item_to_add_event(const string& item, pd::event& event) {
                uint32_t prefix = stoul(item) / YIP_BASE;
                if (prefix < 1 || prefix >= YIP_COUNT) {
                        return false;
                }
                auto yip = static_cast<yunying_item_prefix>(prefix);
                switch (yip) {
                case YIP_RESOURCE:
                        event.set_type(pd::event::ADD_RESOURCE);
                        break;
                case YIP_ITEM:
                        event.set_type(pd::event::ADD_ITEM);
                        break;
                case YIP_EQUIPMENT:
                        event.set_type(pd::event::ADD_EQUIPMENT);
                        break;
                case YIP_HONGMO:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_HONGMO));
                        break;
                case YIP_YANJING:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_YANJING));
                        break;
                case YIP_YANSHENGUANG:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_YANSHENGUANG));
                        break;
                case YIP_SAIHONG:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_SAIHONG));
                        break;
                case YIP_MEIMAO:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_MEIMAO));
                        break;
                case YIP_YANZHUANG:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_YANZHUANG));
                        break;
                case YIP_CHUNCAI:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_CHUNCAI));
                        break;
                case YIP_TIEHUA:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_TIEHUA));
                        break;
                case YIP_LIANXING:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_LIANXING));
                        break;
                case YIP_SPINE_DEFORM_FACE:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_DEFORM_FACE));
                        break;
                case YIP_SPINE_DEFORM_HEAD:
                        event.set_type(pd::event::ADD_SPINE_COLLECTION);
                        event.add_arg(pd::spine_part_Name(pd::SP_DEFORM_HEAD));
                        break;
                case YIP_YIFU:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_YIFU));
                        break;
                case YIP_TOUFA:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_TOUFA));
                        break;
                case YIP_JIEZHI:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_JIEZHI));
                        break;
                case YIP_EDAI:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_EDAI));
                        break;
                case YIP_FAZAN:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_FAZAN));
                        break;
                case YIP_GUANSHI:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_GUANSHI));
                        break;
                case YIP_TOUJIN:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_TOUJIN));
                        break;
                case YIP_ERHUAN:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_ERHUAN));
                        break;
                case YIP_XIANGLIAN:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_XIANGLIAN));
                        break;
                case YIP_BEISHI:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_BEISHI));
                        break;
                case YIP_GUANGHUAN:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_GUANGHUAN));
                        break;
                case YIP_SHOUCHI:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_SHOUCHI));
                        break;
                case YIP_HUANZHUANG_DEFORM_FACE:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_DEFORM_FACE));
                        break;
                case YIP_HUANZHUANG_DEFORM_HEAD:
                        event.set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                        event.add_arg(pd::huanzhuang_part_Name(pd::HP_DEFORM_HEAD));
                        break;
                case YIP_ACTOR_SKIN:
                        event.set_type(pd::event::ACTOR_ADD_SKIN);
                        break;
                default:
                        return false;
                }

                uint32_t id = stoul(item) % YIP_BASE;
                if (event.type() == pd::event::ADD_RESOURCE) {
                        if (!pd::resource_type_IsValid(id)) {
                                return false;
                        }
                        event.add_arg(pd::resource_type_Name(static_cast<pd::resource_type>(id)));
                } else {
                        event.add_arg(to_string(id));
                }

                return true;
        }

        string yunying_serialize_item(uint32_t pttid) {
                return to_string(YIP_ITEM * YIP_BASE + pttid);
        }

        string yunying_serialize_equipment(uint32_t pttid) {
                return to_string(YIP_EQUIPMENT * YIP_BASE + pttid);
        }
        string yunying_serialize_spine(pd::spine_part part, uint32_t pttid) {
                yunying_item_prefix yip = YIP_RESOURCE;
                switch (part) {
                case pd::SP_HONGMO:
                        yip = YIP_HONGMO;
                        break;
                case pd::SP_YANJING:
                        yip = YIP_YANJING;
                        break;
                case pd::SP_YANSHENGUANG:
                        yip = YIP_YANSHENGUANG;
                        break;
                case pd::SP_SAIHONG:
                        yip = YIP_SAIHONG;
                        break;
                case pd::SP_MEIMAO:
                        yip = YIP_MEIMAO;
                        break;
                case pd::SP_YANZHUANG:
                        yip = YIP_YANZHUANG;
                        break;
                case pd::SP_CHUNCAI:
                        yip = YIP_CHUNCAI;
                        break;
                case pd::SP_TIEHUA:
                        yip = YIP_TIEHUA;
                        break;
                case pd::SP_LIANXING:
                        yip = YIP_LIANXING;
                        break;
                case pd::SP_DEFORM_FACE:
                        yip = YIP_SPINE_DEFORM_FACE;
                        break;
                case pd::SP_DEFORM_HEAD:
                        yip = YIP_SPINE_DEFORM_HEAD;
                        break;
                case pd::SP_ZUI:
                        yip = YIP_ZUI;
                        break;
                case pd::SP_HUZI:
                        yip = YIP_HUZI;
                        break;
                case pd::SP_LIAN:
                        yip = YIP_LIAN;
                        break;
                }
                return to_string(yip * YIP_BASE + pttid);
        }

        string yunying_serialize_huanzhuang(pd::huanzhuang_part part, uint32_t pttid) {
                yunying_item_prefix yip = YIP_RESOURCE;
                switch (part) {
                case pd::HP_YIFU:
                        yip = YIP_YIFU;
                        break;
                case pd::HP_TOUFA:
                        yip = YIP_TOUFA;
                        break;
                case pd::HP_JIEZHI:
                        yip = YIP_JIEZHI;
                        break;
                case pd::HP_EDAI:
                        yip = YIP_EDAI;
                        break;
                case pd::HP_FAZAN:
                        yip = YIP_FAZAN;
                        break;
                case pd::HP_GUANSHI:
                        yip = YIP_GUANSHI;
                        break;
                case pd::HP_TOUJIN:
                        yip = YIP_TOUJIN;
                        break;
                case pd::HP_ERHUAN:
                        yip = YIP_ERHUAN;
                        break;
                case pd::HP_XIANGLIAN:
                        yip = YIP_XIANGLIAN;
                        break;
                case pd::HP_BEISHI:
                        yip = YIP_BEISHI;
                        break;
                case pd::HP_GUANGHUAN:
                        yip = YIP_GUANSHI;
                        break;
                case pd::HP_SHOUCHI:
                        yip = YIP_SHOUCHI;
                        break;
                case pd::HP_DEFORM_FACE:
                        yip = YIP_HUANZHUANG_DEFORM_FACE;
                        break;
                case pd::HP_DEFORM_HEAD:
                        yip = YIP_HUANZHUANG_DEFORM_HEAD;
                        break;
                }
                return to_string(yip * YIP_BASE + pttid);
        }

        uint32_t yunying_serialize_resource(pd::resource_type type) {
                return YIP_BASE * YIP_RESOURCE + type;
        }

        pd::money_list_array yunying_money_list() {
                pd::money_list_array ret;
                const auto& resource_ptts = PTTS_GET_ALL(resource);
                for (const auto& i : resource_ptts) {
                        auto *money_list = ret.add_money_list();
                        money_list->set_id(YIP_BASE * YIP_RESOURCE + i.second.type());
                        money_list->set_name(i.second.name());
                        money_list->set_type("2");
                }

                const auto& item_ptts  = PTTS_GET_ALL(item);
                for (const auto& i : item_ptts) {
                        auto *money_list = ret.add_money_list();
                        money_list->set_id(YIP_BASE * YIP_ITEM + i.second.id());
                        money_list->set_name(i.second.name());
                        money_list->set_type("1");
                }

                const auto& equipment_ptts  = PTTS_GET_ALL(equipment);
                for (const auto& i : equipment_ptts) {
                        auto *money_list = ret.add_money_list();
                        money_list->set_id(YIP_BASE * YIP_EQUIPMENT + i.second.id());
                        money_list->set_name(i.second.name());
                        money_list->set_type("1");
                }

                const auto& hongmo_ptts  = PTTS_GET_ALL(hongmo);
                set_money_list(ret, YIP_BASE * YIP_HONGMO, hongmo_ptts);
                const auto& yanjing_ptts  = PTTS_GET_ALL(yanjing);
                set_money_list(ret, YIP_BASE * YIP_YANJING, yanjing_ptts);
                const auto& yanshenguang_ptts  = PTTS_GET_ALL(yanshenguang);
                set_money_list(ret, YIP_BASE * YIP_YANSHENGUANG, yanshenguang_ptts);
                const auto& saihong_ptts  = PTTS_GET_ALL(saihong);
                set_money_list(ret, YIP_BASE * YIP_SAIHONG, saihong_ptts);
                const auto& meimao_ptts  = PTTS_GET_ALL(meimao);
                set_money_list(ret, YIP_BASE * YIP_MEIMAO, meimao_ptts);
                const auto& yanzhuang_ptts  = PTTS_GET_ALL(yanzhuang);
                set_money_list(ret, YIP_BASE * YIP_YANZHUANG, yanzhuang_ptts);
                const auto& chuncai_ptts  = PTTS_GET_ALL(chuncai);
                set_money_list(ret, YIP_BASE * YIP_CHUNCAI, chuncai_ptts);
                const auto& tiehua_ptts  = PTTS_GET_ALL(tiehua);
                set_money_list(ret, YIP_BASE * YIP_TIEHUA, tiehua_ptts);
                const auto& lianxing_ptts  = PTTS_GET_ALL(lianxing);
                set_money_list(ret, YIP_BASE * YIP_LIANXING, lianxing_ptts);

                const auto&  yifu_ptts = PTTS_GET_ALL(yifu);
                set_money_list(ret, YIP_BASE * YIP_YIFU, yifu_ptts);
                const auto& toufa_ptts = PTTS_GET_ALL(toufa);
                set_money_list(ret, YIP_BASE * YIP_TOUFA, toufa_ptts);
                const auto& jiezhi_ptts = PTTS_GET_ALL(jiezhi);
                set_money_list(ret, YIP_BASE * YIP_JIEZHI, jiezhi_ptts);
                const auto&  edai_ptts = PTTS_GET_ALL(edai);
                set_money_list(ret, YIP_BASE * YIP_EDAI, edai_ptts);
                const auto&  fazan_ptts = PTTS_GET_ALL(fazan);
                set_money_list(ret, YIP_BASE * YIP_FAZAN, fazan_ptts);
                const auto&  guanshi_ptts = PTTS_GET_ALL(guanshi);
                set_money_list(ret, YIP_BASE * YIP_GUANSHI, guanshi_ptts);
                const auto&  toujin_ptts = PTTS_GET_ALL(toujin);
                set_money_list(ret, YIP_BASE * YIP_TOUJIN, toujin_ptts);
                const auto&  erhuan_ptts = PTTS_GET_ALL(erhuan);
                set_money_list(ret, YIP_BASE * YIP_ERHUAN, erhuan_ptts);
                const auto&  xianglian_ptts = PTTS_GET_ALL(xianglian);
                set_money_list(ret, YIP_BASE * YIP_XIANGLIAN, xianglian_ptts);
                const auto&  beishi_ptts = PTTS_GET_ALL(beishi);
                set_money_list(ret, YIP_BASE * YIP_BEISHI, beishi_ptts);
                const auto&  guanghuan_ptts = PTTS_GET_ALL(guanghuan);
                set_money_list(ret, YIP_BASE * YIP_GUANGHUAN, guanghuan_ptts);
                const auto&  shouchi_ptts = PTTS_GET_ALL(shouchi);
                set_money_list(ret, YIP_BASE * YIP_SHOUCHI, shouchi_ptts);

                const auto& actor_skin_ptts = PTTS_GET_ALL(actor_skin);
                set_money_list(ret, YIP_BASE * YIP_ACTOR_SKIN, actor_skin_ptts);

                return ret;
        }
}
