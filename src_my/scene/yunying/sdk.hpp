#pragma once

#include "proto/data_base.pb.h"
#include <string>
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class service_thread;
        namespace scene {

                class sdk {
                public:
                        static sdk& instance() {
                                static sdk inst;
                                return inst;
                        }
                        sdk();
                        void init();
                        void init(const shared_ptr<service_thread>& st);
                        void stop();
                        void check_login(const string& token, const function<void(pd::result, const string&)> cb);
                        friend ostream& operator<<(ostream& os, const sdk& ch);
                private:
                        const string name_ = "sdk";
                        shared_ptr<service_thread> st_;
                        FILE *null_;
                };
        }
}
