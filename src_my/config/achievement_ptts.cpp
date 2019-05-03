#include "utils.hpp"
#include "achievement_ptts.hpp"
#include "quest_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                achievement_ptts& achievement_ptts_instance() {
                        static achievement_ptts inst;
                        return inst;
                }

                void achievement_ptts_set_funcs() {
                        achievement_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(quest, ptt.quest())) {
                                        CONFIG_ELOG << "achievement table: " << ptt.id() << " quest not exist " << ptt.quest();
                                }
                                auto& quest_ptt = PTTS_GET(quest, ptt.quest());
                                quest_ptt.set__achievement_quest(ptt.id());
                        };
                }
        }
}
