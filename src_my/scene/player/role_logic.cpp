#include "role_logic.hpp"
#include "scene/player/role.hpp"
#include "scene/condition.hpp"
#include "scene/event.hpp"
#include "config/player_ptts.hpp"
#include "utils/time_utils.hpp"
#include "config/equipment_ptts.hpp"
#include "utils/dirty_word_filter.hpp"
#include "utils/game_def.hpp"
#include "utils/string_utils.hpp"

namespace nora {
        namespace scene {

                pd::result role_check_eat(const role& role) {
                        auto hour = clock::instance().hour();
                        if (hour < 12 ||
                            (hour >= 14 && hour < 18) ||
                            hour > 20) {
                                return pd::ROLE_EAT_WRONG_TIME;
                        }

                        if (system_clock::now() - system_clock::from_time_t(role.eat_time()) <= 2h) {
                                return pd::ROLE_EAT_ALREADY_ATE;
                        }

                        return pd::OK;
                }

                pd::event_res role_eat(role& role) {
                        const auto& ptt = PTTS_GET(role_logic, 1);
                        role.eat();
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_EAT);
                        return event_process(ptt.eat_events(), role, ce);
                }

                pd::result role_check_add_fate(const role& role) {
                        const auto& logic_ptt = PTTS_GET(role_logic, 1);
                        auto ret = condition_check(logic_ptt.add_fate_conditions(), role);
                        if (ret != pd::OK) {
                                return ret;
                        }

                        const auto& ptt = PTTS_GET(role_add_fate, role.fate());
                        return condition_check(ptt.conditions(), role);
                }

                pd::event_res role_add_fate(role& role) {
                        const auto& ptt = PTTS_GET(role_add_fate, role.fate());
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ADD_FATE);
                        ce.add_actors(role.get_role_actor()->pttid());
                        return event_process(ptt.events(), role, ce);
                }

                pd::result role_check_comment(const string& content, const role& role) {
                        return check_content(content);
                }

                pd::result role_comment_by_other(uint64_t from, uint64_t reply_to, const string& content, role& role, uint64_t& gid) {
                        if (reply_to > 0 && !role.has_comment(reply_to)) {
                                return pd::ROLE_COMMENT_NO_SUCH_COMMENT;
                        }
                        if (system_clock::now() - system_clock::from_time_t(role.last_comment_time(from)) < 1min) {
                                return pd::ROLE_COMMENT_IN_CD;
                        }
                        role.add_comment(from, reply_to, content, gid);
                        return pd::OK;
                }

                pd::result role_check_buy_present(uint32_t present, const role& role) {
                        if (role.has_present() && role.get_present() != present) {
                                return pd::ROLE_STILL_HAS_OTHER_PRESENT;
                        }
                        const auto& ptt = PTTS_GET(role_present, present);
                        return condition_check(ptt.buy_conditions(), role);
                }

                pd::event_res role_buy_present(uint32_t present, role& role) {
                        const auto& ptt = PTTS_GET(role_present, present);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ROLE_BUY_PRESENT);
                        return event_process(ptt._buy_events(), role, ce);
                }

                pd::result role_check_upvote_by_other(uint64_t other, const role& role) {
                        if (role.other_upvoted_today(other)) {
                                return pd::ROLE_UPVOTE_ALREADY_UPVOTED_TODAY;
                        }
                        return pd::OK;
                }

                pd::event_res role_upvote_by_other(uint64_t other, role& role, uint32_t& present) {
                        pd::event_res ret;
                        role.upvoted_by_other(other);

                        if (role.has_present()) {
                                const auto& logic_ptt = PTTS_GET(role_logic, 1);
                                if (rand() % 1000 < static_cast<int>(logic_ptt.upvote_get_present_odds())) {
                                        pd::event_array events;
                                        auto *e = events.add_events();
                                        e->set_type(pd::event::DEC_PRESENT);
                                        e->add_arg(to_string(role.get_present()));
                                        e->add_arg(to_string(1));
                                        pd::ce_env ce;
                                        ce.set_origin(pd::CO_ROLE_UPVOTED_GIVE_PRESENT);
                                        ret = event_process(events, role, ce);

                                        present = role.get_present();
                                }
                        }
                        return ret;
                }

                pd::result role_check_share(const role& role) {
                        if (role.share_day() >= refresh_day()) {
                                return pd::ROLE_SHARE_ALREADY_SHARED;
                        }
                        return pd::OK;
                }

                pd::event_res role_share(role& role) {
                        const auto& ptt = PTTS_GET(role_logic, 1);
                        role.share();
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_SHARE);
                        return event_process(ptt.share_events(), role, ce);
                }

                pd::result role_check_change_icon(int icon, const role& role) {
                        const auto& ptt = PTTS_GET(role_logic, 1);
                        auto gender = role.gender();
                        auto icon_count = gender == pd::FEMALE ? ptt.female_icons_size() : ptt.male_icons_size();
                        if (icon < 0 || icon >= icon_count) {
                                return pd::ROLE_INVALID_ICON;
                        }
                        return pd::OK;
                }

                pd::result role_check_change_custom_icon(const pd::custom_icon& custom_icon, const role& role) {
                        const auto& ptt = PTTS_GET(role_logic, 1);
                        auto result = condition_check(ptt.customize_icon_conditions(), role);
                        if (result != pd::OK) {
                                return result;
                        }
                        if (custom_icon.idx() >= PLAYER_CUSTOM_ICON_COUNT) {
                                return pd::ROLE_INVALID_ICON;
                        }
                        if (custom_icon.has_icon() && custom_icon.icon() == "") {
                                return pd::ROLE_INVALID_ICON;
                        }
                        return pd::OK;
                }

                pd::result role_rank_give_like_check(const role& role, uint64_t gid, pd::rank_type type) {
                        if (role.rank_like_check_role(gid, type)) {
                                return pd::RANK_ALREADY_GIVE_ROLE_A_LIKE;
                        }
                        const auto& ptt = PTTS_GET(role_logic, 1);
                        return condition_check(ptt.rank_like_conditions(), role);
                }

                pd::event_res role_rank_give_a_like(role& role) {
                        const auto& ptt = PTTS_GET(role_logic, 1);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_RANK_GIVE_A_LIKE);
                        return event_process(ptt.rank_like_events(), role, ce);
                }

                pd::result role_arena_give_like_check(const role& role, uint64_t gid) {
                        if (role.check_arena_like_role(gid)) {
                                return pd::RANK_ALREADY_GIVE_ROLE_A_LIKE;
                        }
                        const auto& ptt = PTTS_GET(role_logic, 1);
                        return condition_check(ptt.rank_like_conditions(), role); 
                }
        }
}
