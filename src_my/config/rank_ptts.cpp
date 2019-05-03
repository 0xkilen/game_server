#include "utils.hpp"
#include "rank_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                rank_reward_ptts& rank_reward_ptts_instance() {
                        static rank_reward_ptts inst;
                        return inst;
                }

                void rank_reward_ptts_set_funcs() {
                        rank_reward_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.rewards_size() == 0) {
                                        CONFIG_ELOG << ptt.id() << " no rewards";
                                }
                                for (const auto& i : ptt.rewards()) {
                                        check_events(i.events());
                                }
                        };
                        rank_reward_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.rewards()) {
                                        verify_events(i.events());
                                }
                        };
                }

                rank_logic_ptts& rank_logic_ptts_instance() {
                        static rank_logic_ptts inst;
                        return inst;
                }

                void rank_logic_ptts_set_funcs() {
                }

        }
}
