#pragma once
#include <list>
#include <map>
#include "proto_utils.hpp"
using namespace std;

namespace nora {
	template<typename T1, typename T2>
	void list_order_up(list<T1>& order_list, map<T2, typename list<T1>::iterator>& key2iter, T2 key, const T1& new_value) {
		auto order_up_iter = key2iter.at(key);
		typename list<T1>::reverse_iterator rev_iter(order_up_iter);
		while (rev_iter != order_list.rend() && new_value > *rev_iter) {
			++rev_iter;
		}
		auto base_iter = rev_iter.base();
		if (base_iter != order_up_iter) {
			order_list.erase(order_up_iter);
			key2iter[key] = order_list.insert(base_iter, new_value);
		}
		else {
			*order_up_iter = new_value;
		}
	}

	template<typename T1, typename T2>
	void list_order_up(list<T1>& order_list, map<T2, typename list<T1>::iterator>& key2iter, T2 key) {
		auto order_up_iter = key2iter.at(key);
		typename list<T1>::reverse_iterator rev_iter(order_up_iter);
		while (rev_iter != order_list.rend() && *order_up_iter > *rev_iter) {
			++rev_iter;
		}
		auto base_iter = rev_iter.base();
		if (base_iter != order_up_iter) {
			key2iter[key] = order_list.insert(base_iter, *order_up_iter);
			order_list.erase(order_up_iter);
		}
	}
}