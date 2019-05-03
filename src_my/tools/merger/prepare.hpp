#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"
#include "proto/data_league.pb.h"

namespace pd = proto::data;

namespace nora {

        class prepare : public merger_base {
        public:
                static prepare& instance() {
                        static prepare inst;
                        return inst;
                }
                void start();

                const map<uint64_t, string>& from_gid2rolename() const;
                const map<string, uint64_t>& from_rolename2gid() const;
                const map<uint64_t, string>& to_gid2rolename() const;
                const map<string, uint64_t>& to_rolename2gid() const;

                const map<uint64_t, pd::league_separation>& from_gid2league_separation() const;
                const map<string, uint64_t>& from_league_name2gid() const;
                const map<uint64_t, pd::league_separation>& to_gid2league_separation() const;
                const map<string, uint64_t>& to_league_name2gid() const;

        private:
                map<uint64_t, string> from_gid2rolename_;
                map<string, uint64_t> from_rolename2gid_;
                map<uint64_t, string> to_gid2rolename_;
                map<string, uint64_t> to_rolename2gid_;

                map<uint64_t, pd::league_separation> from_gid2league_separation_;
                map<string, uint64_t> from_league_name2gid_;
                map<uint64_t, pd::league_separation> to_gid2league_separation_;
                map<string, uint64_t> to_league_name2gid_;

                int waiting_count_ = 0;
        };

        inline const map<uint64_t, string>& prepare::from_gid2rolename() const {
                return from_gid2rolename_;
        }

        inline const map<string, uint64_t>& prepare::from_rolename2gid() const {
                return from_rolename2gid_;
        }

        inline const map<uint64_t, string>& prepare::to_gid2rolename() const {
                return to_gid2rolename_;
        }

        inline const map<string, uint64_t>& prepare::to_rolename2gid() const {
                return to_rolename2gid_;
        }

        inline const map<uint64_t, pd::league_separation>& prepare::from_gid2league_separation() const {
                return from_gid2league_separation_;
        }

        inline const map<string, uint64_t>& prepare::from_league_name2gid() const {
                return from_league_name2gid_;
        }

        inline const map<uint64_t, pd::league_separation>& prepare::to_gid2league_separation() const {
                return to_gid2league_separation_;
        }

        inline const map<string, uint64_t>& prepare::to_league_name2gid() const {
                return to_league_name2gid_;
        }

}
