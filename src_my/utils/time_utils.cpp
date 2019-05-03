#include "utils/assert.hpp"
#include "time_utils.hpp"
#include "log.hpp"
#include <chrono>
#include <sstream>
#include <sys/time.h>
#include <errno.h>
#include <mutex>

using namespace std;
using namespace chrono;

namespace nora {

        bool add_min(int min) {
                if (min <= 0) {
                        return false;
                }

                struct timeval tv;
                struct timezone tz;
                if (gettimeofday(&tv, &tz) == -1) {
                        ELOG << "gettimeofday failed " << errno;
                        return false;
                }
                tv.tv_sec += min * 60;
                if (settimeofday(&tv, &tz) == -1) {
                        ELOG << "settimeofday failed " << errno;
                        return false;
                }
                DLOG << "add min " << min;
                return true;
        }

        bool add_hour(int hour) {
                if (hour <= 0) {
                        return false;
                }
                return add_min(hour * 60);
        }

        uint32_t refresh_day_from_time_t(uint32_t time) {
                return (duration_cast<hours>(system_clock::from_time_t(time - clock::instance().seconds_west()).time_since_epoch()) - 5h).count() / 24;
        }

        uint32_t refresh_day_to_time_t(uint32_t day) {
                return day * 24 * HOUR + 5 * HOUR + clock::instance().seconds_west();
        }

        int month_days(int month) {
                set<int> big_month { 1, 3, 5, 7, 8, 10, 12 };
                set<int> small_month { 4, 6, 9, 11 };
                if (month == 2) {
                        auto year = clock::instance().year();
                        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
                                return 29;
                        } else {
                                return 28;
                        }
                } else {
                        if (big_month.count(month) > 0) {
                                return 31;
                        } else if (small_month.count(month) > 0) {
                                return 30;
                        }
                }

                ASSERT(false);
                return 0;
        }

        int refresh_year() {
                auto year = clock::instance().year();
                auto month = clock::instance().month();
                auto mday = clock::instance().mday();
                if (mday == 1 && month == 1) {
                        if (clock::instance().hour() < 5) {
                                year -= 1;
                        }
                }
                return year;
        }

        int refresh_month() {
                auto month = clock::instance().month();
                auto mday = clock::instance().mday();
                if (mday == 1) {
                        if (clock::instance().hour() < 5) {
                                month = month - 1 > 0 ? month - 1 : 12;
                        }
                }
                return month;
        }

        int refresh_mday() {
                auto month = clock::instance().month();
                auto mday = clock::instance().mday();

                auto new_month = refresh_month();
                if (month != new_month) {
                        mday = month_days(new_month);
                } else {
                        if (clock::instance().hour() < 5) {
                                mday -= 1;
                        }
                }
                return mday;
        }

        int refresh_wday() {
                auto wday = clock::instance().wday();
                auto hour = clock::instance().hour();
                if (hour < 5) {
                        wday -= 1;
                        if (wday == 0) {
                                wday = 7;
                        }
                }
                return wday;
        }

        int refresh_monday() {
                auto wday = clock::instance().wday();
                ASSERT(wday > 0);
                auto hour = clock::instance().hour();
                if (hour < 5) {
                        wday -= 1;
                        if (wday == 0) {
                                wday = 7;
                        }
                }
                return refresh_day() + 1 - wday;
        }


        void clock::start(const shared_ptr<service_thread>& st) {
                st_ = st;
                stop_.store(false);
                update();
                add_update_timer();
        }

        void clock::stop() {
                stop_.store(true);
        }

        void clock::add_update_timer() {
                ADD_TIMER(
                        st_,
                        ([this] (auto canceled, const auto& timer) {
                                if (!canceled && !stop_.load()) {
                                        this->update();
                                        this->add_update_timer();
                                }
                        }),
                        50ms);
        }

        void clock::update() {
                lock_guard<mutex> lock(lock_);
                auto tp = high_resolution_clock::now();
                ms_ = duration_cast<milliseconds>(tp.time_since_epoch());
                auto tt = duration_cast<seconds>(ms_).count();
                auto old_tm = tm_;
                tm_ = *localtime(&tt);

                if (old_tm.tm_sec != tm_.tm_sec && tm_.tm_sec == 0) {
                        on_min(tm_.tm_min);
                        if (old_tm.tm_min != tm_.tm_min && tm_.tm_min == 0) {
                                on_hour(tm_.tm_hour);
                        }
                }
        }

        string clock::now_date_str() {
                lock_guard<mutex> lock(lock_);
                ostringstream oss;
                const size_t size = 512;
                char buffer[size] = {0};
                auto strsize = strftime(buffer, size, "%F", &tm_);
                if (0 == strsize) {
                        return "";
                }
                oss.write(buffer, strsize);
                return oss.str();
        }

        string clock::now_time_str(bool usec) {
                lock_guard<mutex> lock(lock_);
                ostringstream oss;
                const size_t size = 512;
                char buffer[size] = {0};
                auto strsize = strftime(buffer, size, "%F %T", &tm_);
                if (0 == strsize) {
                        return "";
                }
                oss.write(buffer, strsize);
                if (usec) {
                        oss << '.' << setw(3) << setfill('0') << ms_.count() % 1000;
                }
                return oss.str();
        }

        string clock::time_str(time_t tt) {
                lock_guard<mutex> lock(lock_);
                ostringstream oss;
                const size_t size = 512;
                char buffer[size] = {0};
                auto strsize = strftime(buffer, size, "%F %T", localtime(&tt));
                if (0 == strsize) {
                        return "";
                }
                oss.write(buffer, strsize);
                return oss.str();
        }

        list<function<void()>>::iterator clock::register_hour_cb(int hour, const function<void()>& cb) {
                hour_cbs_[hour].push_front(cb);
                return hour_cbs_[hour].begin();
        }

        list<function<void()>>::iterator clock::register_min_cb(int min, const function<void()>& cb) {
                min_cbs_[min].push_front(cb);
                return min_cbs_[min].begin();
        }

        void clock::on_hour(int hour) {
                if (hour_cbs_.count(hour) > 0) {
                        DLOG << "exec hour[" <<  hour << "] cb, count:" << hour_cbs_.at(hour).size();
                        for (const auto& i : hour_cbs_.at(hour)) {
                                st_->async_call(
                                        [i] {
                                                i();
                                        });
                        }
                }
        }

        void clock::on_min(int min) {
                if (min_cbs_.count(min) > 0) {
                        DLOG << "exec min[" <<  min << "] cb, count:" << min_cbs_.at(min).size();
                        for (const auto& i : min_cbs_.at(min)) {
                                st_->async_call(
                                        [i] {
                                                i();
                                        });
                        }
                }
        }

}
