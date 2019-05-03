#pragma once

#include "service_thread.hpp"
#include "proto/data_base.pb.h"
#include <string>
#include <chrono>
#include <memory>
#include <map>
#include <list>
#include <atomic>

using namespace std;
using namespace chrono;
namespace pd = proto::data;
namespace ba = boost::asio;

#define MINUTE 60
#define HOUR (MINUTE * 60)
#define DAY (HOUR * 24)

namespace nora {

        bool add_min(int min);
        bool add_hour(int hour);

        uint32_t refresh_day_from_time_t(uint32_t time);
        uint32_t refresh_day_to_time_t(uint32_t day);

        int month_days(int month);

        int refresh_year();
        int refresh_month();
        int refresh_mday();
        int refresh_wday();
        int refresh_monday();

        inline time_t steady_clock_to_time_t(steady_clock::time_point tp) {
                return system_clock::to_time_t(system_clock::now() + (tp - steady_clock::now()));
        }

        inline uint32_t refresh_day() {
                time_t now_time_t = system_clock::to_time_t(system_clock::now());
                return refresh_day_from_time_t(now_time_t);
        }

        inline bool operator==(const pd::time_point& a, const pd::time_point& b) {
                return a.hour() == b.hour() && a.min() == b.min();
        }

        inline bool operator>(const pd::time_point& a, const pd::time_point& b) {
                if (a.hour() > b.hour()) {
                        return true;
                } else if (a.hour() == b.hour() && a.min() > b.min()) {
                        return true;
                }
                return false;
        }

        inline bool operator>=(const pd::time_point& a, const pd::time_point& b) {
                if (a.hour() > b.hour()) {
                        return true;
                } else if (a.hour() == b.hour() && a.min() >= b.min()) {
                        return true;
                }
                return false;
        }

        inline bool operator<(const pd::time_point& a, const pd::time_point& b) {
                if (a.hour() < b.hour()) {
                        return true;
                } else if (a.hour() == b.hour() && a.min() < b.min()) {
                        return true;
                }
                return false;
        }

        struct time_point_cmp {
                bool operator()(const pd::time_point& a, const pd::time_point& b) const {
                        if (a.hour() < b.hour()) {
                                return true;
                        } else if (a.hour() == b.hour() && a.min() < b.min()) {
                                return true;
                        }
                        return false;
                }
        };

        inline pd::time_period times_to_period(const pd::time_point& begin, const pd::time_point& end) {
                pd::time_period ret;
                *ret.mutable_begin() = begin;
                *ret.mutable_end() = end;
                return ret;
        }

        inline pd::time_point min_after_time_point(const pd::time_point& a, int min) {
                auto m = a.min() + min;
                auto h = a.hour();
                if (m >= 60) {
                        h += m / 60;
                        m = m % 60;
                }
                if (h >= 24) {
                        h = h % 24;
                }
                pd::time_point ret;
                ret.set_min(m);
                ret.set_hour(h);
                return ret;
        }

        inline bool in_time_period(const pd::time_point& time, const pd::time_period& period) {
                if (time.hour() < period.begin().hour()) {
                        return false;
                } else if (time.hour() == period.begin().hour() && time.min() < period.begin().min()) {
                        return false;
                }

                if (time.hour() > period.end().hour()) {
                        return false;
                } else if (time.hour() == period.end().hour() && time.min() >= period.end().min()) {
                        return false;
                }

                return true;
        }

        inline pd::time_point min_before_time_point(const pd::time_point& a, int min) {
                auto m = a.min();
                auto h = a.hour();

                while (m < min) {
                        h -= 1;
                        if (h < 0) {
                                h += 24;
                        }
                        m += 60;
                }
                m -= min;

                pd::time_point ret;
                ret.set_min(m);
                ret.set_hour(h);
                return ret;
        }

        class clock {
        public:
                static clock& instance() {
                        static clock inst;
                        return inst;
                }
                void start(const shared_ptr<service_thread>& st);
                void stop();

                list<function<void()>>::iterator register_min_cb(int minute, const function<void()>& cb);
                list<function<void()>>::iterator register_hour_cb(int hour, const function<void()>& cb);

                string now_date_str();
                string now_time_str(bool usec = false);
                string time_str(time_t tt);

                int sec() const;
                int min() const;
                int hour() const;
                int year() const;
                int month() const;
                int mday() const;
                int wday() const;
                long int seconds_west() const;
                pd::time_point now() const;
        private:
                void add_update_timer();
                void on_hour(int hour);
                void on_min(int min);
                void update();

                map<int, list<function<void()>>> min_cbs_;
                map<int, list<function<void()>>> hour_cbs_;
                shared_ptr<service_thread> st_;

                mutable mutex lock_;
                tm tm_;
                milliseconds ms_;
                atomic<bool> stop_;
        };

        inline int clock::sec() const {
                lock_guard<mutex> lock(lock_);
                return tm_.tm_sec;
        }

        inline int clock::min() const {
                lock_guard<mutex> lock(lock_);
                return tm_.tm_min;
        }

        inline int clock::hour() const {
                lock_guard<mutex> lock(lock_);
                return tm_.tm_hour;
        }

        inline int clock::year() const {
                lock_guard<mutex> lock(lock_);
                return tm_.tm_year + 1900;
        }

        inline int clock::month() const {
                lock_guard<mutex> lock(lock_);
                return tm_.tm_mon + 1;
        }

        inline int clock::mday() const {
                lock_guard<mutex> lock(lock_);
                return tm_.tm_mday;
        }

        inline int clock::wday() const {
                lock_guard<mutex> lock(lock_);
                return tm_.tm_wday == 0 ? 7 : tm_.tm_wday;
        }

        inline pd::time_point clock::now() const {
                pd::time_point ret;
                ret.set_hour(hour());
                ret.set_min(min());
                return ret;
        }

        inline long int clock::seconds_west() const {
                lock_guard<mutex> lock(lock_);
                return -tm_.tm_gmtoff + tm_.tm_isdst * HOUR;
        }

}
