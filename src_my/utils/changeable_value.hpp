#pragma once

namespace nora {

        class changeable_value {
        public:
                void init(int64_t base);
                void change_value(double value);
                void change_percent(double percent);
                int64_t value() const;
                int64_t base() const;
        private:
                int64_t base_ = 0;
                double change_value_ = 0.0;
                double change_percent_ = 0.0;
        };

        inline void changeable_value::init(int64_t base) {
                base_ = base;
                change_value_ = 0.0;
                change_percent_ = 0.0;
        }

        inline void changeable_value::change_value(double value) {
                change_value_ += value;
        }

        inline void changeable_value::change_percent(double percent) {
                change_percent_ += percent;
        }

        inline int64_t changeable_value::value() const {
                return static_cast<int64_t>((base_ + change_value_) * (1000 + change_percent_) / 1000.0);
        }

        inline int64_t changeable_value::base() const {
                return base_;
        }

}
