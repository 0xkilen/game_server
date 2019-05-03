#include "utils/functions.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <string>

namespace nora {
namespace test {

        class functions_test : public CppUnit::TestFixture {
                CPPUNIT_TEST_SUITE(functions_test);
                CPPUNIT_TEST(test);
                CPPUNIT_TEST_SUITE_END();
        public:
                void test();
        };
        
        CPPUNIT_TEST_SUITE_REGISTRATION(functions_test);

        void functions_test::test() {
                int a = 0;
                functions<void> funcs;

                funcs.add([&a] {a++;});
                funcs();
                CPPUNIT_ASSERT_EQUAL(1, a);

                auto fi = funcs.add([&a] {a++;});
                funcs();
                CPPUNIT_ASSERT_EQUAL(3, a);

                funcs.remove(fi);
                funcs();
                CPPUNIT_ASSERT_EQUAL(4, a);
        }

}
}
