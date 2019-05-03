#include "proto/data_actor.pb.h"
#include "config/utils.hpp"
#include "config/buff_ptts.hpp"
#include "config/common_ptts.hpp"
#include "config/actor_ptts.hpp"
#include <map>
#include <mutex>
#include <string>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                uint32_t calc_zhanli_by_attrs(const map<pd::actor::attr_type, int64_t>& attrs) {
                        static const map<pd::actor::attr_type, double> attr2factor_ {
                                { pd::actor::HP, 0.1 },
                                { pd::actor::ATTACK, 1.0 },
                                { pd::actor::PHYSICAL_DEFENCE, 0.5 },
                                { pd::actor::MAGICAL_DEFENCE, 0.5 },
                                { pd::actor::HIT_RATE, 8.0 },
                                { pd::actor::DODGE_RATE, 8.0 },
                                { pd::actor::BOOM_RATE, 4.0 },
                                { pd::actor::ANTI_BOOM, 4.0 },
                                { pd::actor::BOOM_DAMAGE, 1.6 },
                                { pd::actor::OUT_DAMAGE_PERCENT, 9.6 },
                                { pd::actor::IN_DAMAGE_PERCENT, 9.6 },
                                { pd::actor::OUT_HEAL_PERCENT, 9.6 },
                                { pd::actor::IN_HEAL_PERCENT, 6 },
                                        };
                        double ret = 0.0;
                        for (const auto& i : attrs) {
                                switch (i.first) {
                                case pd::actor::HP:
                                case pd::actor::ATTACK:
                                case pd::actor::PHYSICAL_DEFENCE:
                                case pd::actor::MAGICAL_DEFENCE:
                                case pd::actor::BOOM_RATE:
                                case pd::actor::ANTI_BOOM:
                                case pd::actor::DODGE_RATE:
                                case pd::actor::BOOM_DAMAGE:
                                case pd::actor::HIT_RATE:
                                case pd::actor::OUT_DAMAGE_PERCENT:
                                case pd::actor::IN_DAMAGE_PERCENT:
                                case pd::actor::OUT_HEAL_PERCENT:
                                case pd::actor::IN_HEAL_PERCENT:
                                        ret += i.second * attr2factor_.at(i.first);
                                        break;
                                default:
                                        break;
                                }
                        }
                        return static_cast<uint32_t>(ret);
                }

                const set<string>& forbid_names() {
                        static set<string> ret;
                        static mutex lock;
                        lock_guard<mutex> lk(lock);
                        if (ret.empty()) {
                                const auto& ptt = PTTS_GET(common, 1);
                                for (auto i : ptt.forbid_name_actors()) {
                                        const auto& actor_ptt = PTTS_GET(actor, i);
                                        ret.insert(actor_ptt.name());
                                }
                        }
                        return ret;
                }

        }
}
