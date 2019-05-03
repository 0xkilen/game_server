#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include <map>
#include <functional>
#include <exception>
#include <chrono>

using namespace std;
using namespace chrono;
using namespace google::protobuf;

namespace proto {

        template <typename T, typename BASE>
        class processor {
        public:
                static void register_proto_handler(const Descriptor *descriptor, function<void(T *, const BASE *)> handler) {
                        handlers_[descriptor] = handler;
                }
                bool has_handler(T *t, const BASE *proto) const {
                        const Reflection *reflection = proto->GetReflection();
                        vector<const FieldDescriptor *> fields;
                        reflection->ListFields(*proto, &fields);
                        if (fields.size() > 2 || fields.size() < 1) {
                                ELOG << *t << " got proto with " << fields.size() << " fields";
                                return false;
                        }

                        const Descriptor *desc = fields.back()->message_type();
                        return handlers_.count(desc) > 0;
                }
                void process(T *t, const BASE *proto) {
                        const Reflection *reflection = proto->GetReflection();
                        vector<const FieldDescriptor *> fields;
                        reflection->ListFields(*proto, &fields);
                        if (fields.size() > 2 || fields.size() < 1) {
                                ELOG << *t << " got proto with " << fields.size() << " fields";
                                return;
                        }

                        const Descriptor *desc = fields.back()->message_type();
                        if (handlers_.count(desc) <= 0) {
                                return;
                        }

                        if (ignore_min_frequency_.count(desc) == 0 &&
                            min_frequency_ > system_clock::duration::zero()) {
                                auto now = system_clock::now();
                                if (last_proto_times_.count(desc) > 0 && now - last_proto_times_.at(desc) < min_frequency_) {
                                        return;
                                }
                                last_proto_times_[desc] = now;
                        }

                        if (process_msg_cb_) {
                                process_msg_cb_(desc, proto->ByteSize());
                        }

                        try {
                                handlers_[desc](t, proto);
                        } catch(const exception& e) {
                                ELOG << e.what() << "\nproto:\n" << proto->DebugString();
                        }
                }

                void set_proto_min_frequency(const system_clock::duration& du) {
                        min_frequency_ = du;
                }

                void add_ignore_min_frequency(const Descriptor *desc) {
                        ignore_min_frequency_.insert(desc);
                }

                function<void(const Descriptor *, size_t)> process_msg_cb_;
        private:
                static map<const Descriptor *, function<void(T *, const BASE *)>> handlers_;
                system_clock::duration min_frequency_ = system_clock::duration::zero();
                map<const Descriptor *, system_clock::time_point> last_proto_times_;
                set<const Descriptor *> ignore_min_frequency_;
        };

        template <typename T, typename BASE>
        map<const Descriptor *, function<void(T *, const BASE *)>> processor<T, BASE>::handlers_;

}
