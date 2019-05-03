#include "gongdou_ptts.hpp"
#include "title_ptts.hpp"
#include "utils.hpp"
#include "mail_ptts.hpp"
#include "chat_ptts.hpp"

namespace nora {
        namespace config {

                gongdou_ptts& gongdou_ptts_instance() {
                        static gongdou_ptts inst;
                        return inst;
                }

                void gongdou_ptts_set_funcs() {
                        gongdou_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " check conditions failed";
                                }
                                if (!check_conditions(ptt.target_conditions())) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " check target_conditions failed";
                                }
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " check events failed";
                                }
                                if (!check_events(ptt.target_events())) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " check target_events failed";
                                }
                        };
                        gongdou_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable_events());
                        };
                        gongdou_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " verify conditions failed";
                                }
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " verify events failed";
                                }
                                if (!verify_events(ptt.target_events())) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " verify target_events failed";
                                }
                                for (const auto& i : ptt.deform().male_pools()) {
                                        for (const auto& j : i.spine()) {
                                                if (!SPINE_PTTS_HAS(j.part(), j.pttid())) {
                                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " deform spine not exist " << pd::spine_part_Name(j.part()) << " " << j.pttid();
                                                }
                                        }
                                        for (const auto& j : i.huanzhuang()) {
                                                if (!HUANZHUANG_PTTS_HAS(j.part(), j.pttid())) {
                                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " deform huanzhuang not exist " << pd::huanzhuang_part_Name(j.part()) << " " << j.pttid();
                                                }
                                        }
                                }
                                for (const auto& i : ptt.deform().female_pools()) {
                                        for (const auto& j : i.spine()) {
                                                if (!SPINE_PTTS_HAS(j.part(), j.pttid())) {
                                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " deform spine not exist " << pd::spine_part_Name(j.part()) << " " << j.pttid();
                                                }
                                        }
                                        for (const auto& j : i.huanzhuang()) {
                                                if (!HUANZHUANG_PTTS_HAS(j.part(), j.pttid())) {
                                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " deform huanzhuang not exist " << pd::huanzhuang_part_Name(j.part()) << " " << j.pttid();
                                                }
                                        }
                                }
                                for (const auto& i : ptt.slander()) {
                                        if (!PTTS_HAS(title, i.title())) {
                                                CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " slander title not exist " << i.title();
                                        }
                                        if (!PTTS_HAS(system_chat, i.system_chat())) {
                                                CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " slander system_chat not exist " << i.system_chat();
                                        }
                                }
                                if (ptt.type() == pd::GT_SLANDER && ptt.slander_size() < 2) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " title pool need at least 2 titles";
                                }
                                if (ptt.type() == pd::GT_SORCERY && ptt.sorcery().female_icon_pool_size() < 2) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " female icon pool need at least 2 icons";
                                }
                                if (ptt.type() == pd::GT_SORCERY && ptt.sorcery().male_icon_pool_size() < 2) {
                                        CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " male icon pool need at least 2 icons";
                                }
                                if (ptt.has_gongdou_by_other_mail()) {
                                        if (!PTTS_HAS(mail, ptt.gongdou_by_other_mail())) {
                                                CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " gonggou_by_other_mail not exist " << ptt.gongdou_by_other_mail();
                                        }
                                }
                                if (ptt.has_system_chat()) {
                                        if (!PTTS_HAS(system_chat, ptt.system_chat())) {
                                                CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " has no system_chat";
                                        }
                                }
                                for (auto i : ptt.buffs()) {
                                        if (!PTTS_HAS(buff, i)) {
                                                CONFIG_ELOG << pd::gongdou_type_Name(ptt.type()) << " buff not exist " << i;
                                        }
                                }
                        };
                }

        }
}
