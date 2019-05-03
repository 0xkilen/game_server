#pragma once

#include <google/protobuf/message.h>
#include <vector>

using namespace std;
using namespace google::protobuf;

namespace nora {
        namespace dbcache {

                template <typename T>
                void dbcache_update(T& cached_data, const T& updated_data) {
                        const Reflection* cached_reflection = cached_data.GetReflection();
                        const Reflection* updated_reflection = updated_data.GetReflection();
                        vector<const FieldDescriptor*> fields;
                        updated_reflection->ListFields(updated_data, &fields);
                        for (size_t idx = 0; idx < fields.size(); ++idx) {
                                if (fields[idx]->is_optional() &&
                                    fields[idx]->type() == FieldDescriptor::TYPE_MESSAGE) {
                                        const auto& sub_msg = updated_reflection->GetMessage(updated_data, fields[idx]);
                                        cached_reflection->MutableMessage(&cached_data, fields[idx])->CopyFrom(sub_msg);
                                }
                        }
                }

        }
}
