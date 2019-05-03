#include "utils/time_utils.hpp"
#include "utils/service_thread.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <atomic>
#include <thread>

using namespace std;

namespace nora {
        namespace test {

                class time_utils_test : public CppUnit::TestFixture {
                        CPPUNIT_TEST_SUITE(time_utils_test);
                        CPPUNIT_TEST(test);
                        CPPUNIT_TEST_SUITE_END();
                public:
                        void test();
                        void time_part_test();
                };

                CPPUNIT_TEST_SUITE_REGISTRATION(time_utils_test);

                void time_utils_test::test() {
                        atomic<int> done_num{0};
                        vector<shared_ptr<service_thread>> sts;
                        for (int i = 0; i < 8; ++i) {
                                sts.push_back(make_shared<service_thread>("time_utils"));
                        }
                        for_each(sts.begin(), sts.end(), [] (const auto& st) {
                                        st->start();
                                });
                        for_each(sts.begin(), sts.end(), [&done_num] (const auto& st){
                                        st->async_call(
                                                [&done_num] {
                                                        for (int i = 0; i < 3000; ++i) {
                                                                clock::instance().now_time_str();
                                                        }
                                                        done_num++;
                                                });
                                });
                        while (done_num.load() < 8) {
                                continue;
                        }
                        for_each(sts.begin(), sts.end(),
                                 [] (auto& st) {
                                         st->stop();
                                 });
                        sts.clear();
                }

        }
}
