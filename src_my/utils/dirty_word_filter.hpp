#pragma once

#include <map>
#include <memory>

using namespace std;

namespace nora {

        struct trie_node {
                bool complete_ = false;
                map<char, shared_ptr<trie_node>> children_;
        };

        class dirty_word_filter {
        public:
                static dirty_word_filter& instance() {
                        static dirty_word_filter inst;
                        return inst;
                }
                void insert(const string& word);
                void clear();
                bool check(const string& sentence) const;
                string fix(const string& sentence, char sub = '*') const;
        private:
                void fix(const char *start, size_t len, char sub, string& ret) const;

                shared_ptr<trie_node> trie_root_ = make_shared<trie_node>();
        };

}
