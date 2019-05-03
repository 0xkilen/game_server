#include "utils/atom.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <string>

namespace nora {
namespace test {

        class atom_test : public CppUnit::TestFixture {
                CPPUNIT_TEST_SUITE(atom_test);
                CPPUNIT_TEST(test);
                CPPUNIT_TEST_SUITE_END();
        public:
                void test();
        };
        
        CPPUNIT_TEST_SUITE_REGISTRATION(atom_test);

        void atom_test::test() {
                atom a1 = atom_emplace("hello");
                atom a2 = atom_emplace("hello");
                atom a3 = atom_emplace("hellohello");
                CPPUNIT_ASSERT(a1 == a2);
                CPPUNIT_ASSERT(a1 != a3);

                for (int i = 0; i < 1000000; ++i) {
                        atom_emplace("hello" + std::to_string(i));
                }

                atom a4 = atom_emplace("hello");
                CPPUNIT_ASSERT(a1 == a4);
        }
}
}
