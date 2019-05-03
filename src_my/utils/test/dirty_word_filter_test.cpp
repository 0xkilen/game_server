#include "utils/dirty_word_filter.hpp"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

namespace nora {
        namespace test {

                class dirty_word_filter_test : public CppUnit::TestFixture {
                        CPPUNIT_TEST_SUITE(dirty_word_filter_test);
                        CPPUNIT_TEST(test_check);
                        CPPUNIT_TEST(test_fix);
                        CPPUNIT_TEST(test_fix_as_long_as_possible);
                        CPPUNIT_TEST(test_fix_as_many_as_possible);
                        CPPUNIT_TEST_SUITE_END();
                public:
                        void tearDown() override;
                        void test_check();
                        void test_fix();
                        void test_fix_as_long_as_possible();
                        void test_fix_as_many_as_possible();
                };

                CPPUNIT_TEST_SUITE_REGISTRATION(dirty_word_filter_test);

                void dirty_word_filter_test::tearDown() {
                        auto& dwf = dirty_word_filter::instance();
                        dwf.clear();
                }

                void dirty_word_filter_test::test_check() {
                        auto& dwf = dirty_word_filter::instance();
                        CPPUNIT_ASSERT(dwf.check("我是程序员"));
                        dwf.insert("程序");
                        CPPUNIT_ASSERT(!dwf.check("我是程序员"));
                        CPPUNIT_ASSERT(dwf.check("我是策划"));
                }

                void dirty_word_filter_test::test_fix() {
                        auto& dwf = dirty_word_filter::instance();
                        dwf.insert("程序");

                        string str("我是程序员");
                        str = dwf.fix(str);
                        CPPUNIT_ASSERT_EQUAL(string("我是**员"), str);
                }

                void dirty_word_filter_test::test_fix_as_long_as_possible() {
                        auto& dwf = dirty_word_filter::instance();
                        dwf.insert("程序");
                        dwf.insert("程序员");

                        string str("我是程序员我写代码");
                        str = dwf.fix(str);
                        CPPUNIT_ASSERT_EQUAL(string("我是***我写代码"), str);
                }

                void dirty_word_filter_test::test_fix_as_many_as_possible() {
                        auto& dwf = dirty_word_filter::instance();
                        dwf.insert("程序员");
                        dwf.insert("写代码");

                        string str("我是程序员我写代码");
                        str = dwf.fix(str);
                        CPPUNIT_ASSERT_EQUAL(string("我是***我***"), str);
                }

        }
}
