#include "utils.hpp"
#include "relation_ptts.hpp"
#include "item_ptts.hpp"
#include "chat_ptts.hpp"

namespace nora {
        namespace config {

                relation_logic_ptts& relation_logic_ptts_instance() {
                        static relation_logic_ptts inst;
                        return inst;
                }

                void relation_logic_ptts_set_funcs() {
                }

                relation_suggestion_ptts& relation_suggestion_ptts_instance() {
                        static relation_suggestion_ptts inst;
                        return inst;
                }

                void relation_suggestion_ptts_set_funcs() {
                }

                relation_gift_ptts& relation_gift_ptts_instance() {
                        static relation_gift_ptts inst;
                        return inst;
                }

                void relation_gift_ptts_set_funcs() {
                        relation_gift_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(item, ptt.item())) {
                                        CONFIG_ELOG <<" gift item not exist " << ptt.item();
                                }
                                if (!PTTS_HAS(system_chat, ptt.system_chat())) {
                                        CONFIG_ELOG <<" gift system chat not exist " << ptt.system_chat();
                                }
                        };
                }

        }
}
