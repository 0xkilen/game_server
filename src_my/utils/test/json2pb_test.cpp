#include "utils/json2pb.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <iostream>
#include "proto/json2pb_test.pb.h"

using namespace std;

namespace nora {
namespace test {

        class json2pb_test : public CppUnit::TestFixture {
                CPPUNIT_TEST_SUITE(json2pb_test);
                CPPUNIT_TEST(test);
                CPPUNIT_TEST_SUITE_END();
        public:
                void test();
        };

        CPPUNIT_TEST_SUITE_REGISTRATION(json2pb_test);

        void json2pb_test::test() {
                proto::test::msg m;

                m.set_u32(1);
                m.set_dou(1.5);
                auto nm = m.mutable_nm();
                nm->add_rstr("hello");
                nm->add_rstr("world");
                auto *rnm = m.add_rnm();
                rnm->add_rstr("welcome");
                rnm->add_rstr("pb");
                rnm = m.add_rnm();
                rnm->add_rstr("welcome");
                rnm->add_rstr("json");

                CPPUNIT_ASSERT_EQUAL(string("{\"u32\": 1, \"dou\": 1.5, \"nm\": {\"rstr\": [\"hello\", \"world\"]}, \"rnm\": [{\"rstr\": [\"welcome\", \"pb\"]}, {\"rstr\": [\"welcome\", \"json\"]}]}"), pb2json(m));
        }
}
}
