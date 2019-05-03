#include "utils/gid.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <set>
#include <thread>

using namespace std;

namespace nora {
        namespace test {

                class gid_test : public CppUnit::TestFixture {
                        CPPUNIT_TEST_SUITE(gid_test);
                        CPPUNIT_TEST(test_mt);
                        CPPUNIT_TEST_SUITE_END();
                public:
                        void setUp() override;
                        void test_mt();
                };

                CPPUNIT_TEST_SUITE_REGISTRATION(gid_test);

                void gid_test::setUp() {
                        gid::instance().set_server_id(0);
                }

                void gid_test::test_mt() {
                        int count = 5000;
                        int thread_count = 10;
                        vector<thread> ts;

                        try {
                                for (int i = 0; i < thread_count; ++i) {
                                        ts.push_back(move(thread([&] {
                                                                        for (int i = 0; i < count; ++i) {
                                                                                auto gid = gid::instance().new_gid(gid_type::ROLE);
                                                                                CPPUNIT_ASSERT(gid_type::ROLE == gid::instance().get_type(gid));
                                                                        }
                                                                        for (int i = 0; i < count; ++i) {
                                                                                auto gid = gid::instance().new_gid(gid_type::ROLE);
                                                                                CPPUNIT_ASSERT(gid_type::ROLE == gid::instance().get_type(gid));
                                                                        }
                                                                })));
                                }
                        } catch (const exception& e) {
                                for (auto&& i : ts) {
                                        i.join();
                                }
                        }

                        for (auto&& i : ts) {
                                i.join();
                        }
                }

        }
}
