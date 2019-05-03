#include "dirty_word_filter.hpp"

namespace nora {

        void dirty_word_filter::insert(const string& word) {
                if (word.empty()) {
                        return;
                }

                auto cur_node = trie_root_;
                for (auto i : word) {
                        if (cur_node->children_.count(i) <= 0) {
                                cur_node->children_.emplace(i, make_shared<trie_node>());
                        }
                        cur_node = cur_node->children_[i];
                }
                cur_node->complete_ = true;
        }

        void dirty_word_filter::clear() {
                trie_root_->children_.clear();
        }

        bool dirty_word_filter::check(const string& sentence) const {
                if (sentence.empty()) {
                        return true;
                }

                auto cur_node = trie_root_;
                for (auto i : sentence) {
                        if (cur_node->children_.count(i) <= 0) {
                                if (sentence.size() == 1) {
                                        return true;
                                } else {
                                        return check(sentence.substr(1));
                                }
                        }
                        cur_node = cur_node->children_.at(i);
                        if (cur_node->complete_) {
                                return false;
                        }
                }

                return true;
        }

        void dirty_word_filter::fix(const char *start, size_t len, char sub, string& ret) const {
                auto found_length = 0;
                auto length = 0;
                auto cur_node = trie_root_;
                auto *cur = start;
                auto cur_len = len;
                while (cur_len > 0) {
                        auto c = *cur;
                        if (cur_node->children_.count(c) <= 0) {
                                break;
                        }
                        length += 1;
                        cur_node = cur_node->children_.at(c);
                        if (cur_node->complete_) {
                                found_length = length;
                        }

                        cur++;
                        cur_len -= 1;
                }

                cur = start;
                while (found_length > 0) {
                        if ((*cur & 0xc0) != 0x80) {
                                ret += sub;
                        }
                        found_length -= 1;
                        cur++;
                        len -= 1;
                }

                if (len <= 0) {
                        return;
                } else if (cur == start) {
                        ret += *cur;
                        cur = start + 1;
                        len -= 1;
                }
                fix(cur, len, sub, ret);
        }

        string dirty_word_filter::fix(const string& sentence, char sub) const {
                string ret;
                if (sentence.empty()) {
                        return ret;
                }

                fix(&sentence[0], sentence.size(), sub, ret);
                return ret;
        }


}
