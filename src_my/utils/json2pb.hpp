#pragma once

#include <string>

using namespace std;
namespace google {
namespace protobuf {
        class Message;
}
}

namespace nora {

        void json2pb(google::protobuf::Message &msg, const char *buf, size_t size);
        string pb2json(const google::protobuf::Message &msg);

}

