#include "sdk.hpp"
#include "utils/service_thread.hpp"
#include "utils/yunying.hpp"
#include "utils/assert.hpp"
#include "utils/time_utils.hpp"
#include "config/options_ptts.hpp"
#include <curl/curl.h>
#include <map>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace bpt = boost::property_tree;

#define SSDK_DLOG __DLOG << setw(20) << "[SCENESDK] "
#define SSDK_ILOG __ILOG << setw(20) << "[SCENESDK] "
#define SSDK_ELOG __ELOG << setw(20) << "[SCENESDK] "

namespace {

        static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
                auto *tree = (bpt::ptree *)userp;
                size_t realsize = size * nmemb;
                if (realsize > 0) {
                        string str((char *)contents, realsize);
                        istringstream iss(str);
                        read_json(iss, *tree);
                }
                return realsize;
        }

}

namespace nora {
        namespace scene {

                sdk::sdk() {
                        null_ = fopen("/dev/null", "wb");
                }

                void sdk::init() {
                        st_ = make_shared<service_thread>("yunying sdk");
                        st_->start();
                }

                void sdk::init(const shared_ptr<service_thread>& st) {
                        ASSERT(st);
                        st_ = st;
                }

                void sdk::stop() {
                        if (st_) {
                                st_->stop();
                                st_.reset();
                        }
                        SSDK_ILOG << "sdk stop";
                }

                void sdk::check_login(const string& token, const function<void(pd::result, const string&)> cb) {
                        ASSERT(st_);
                        st_->async_call(
                                [this, token, cb] {
                                        auto ptt = PTTS_GET_COPY(options, 1);

                                        map<string, string> params;
                                        params["appId"] = ptt.yunying().app_id();
                                        params["openId"] = token;
                                        params["sign"] = yunying_calc_sign(params, ptt.yunying().app_secret());

                                        string content;
                                        for (const auto& i : params) {
                                                if (!content.empty()) {
                                                        content += '&';
                                                }
                                                content += i.first;
                                                content += '=';
                                                content += i.second;
                                        }

                                        SSDK_DLOG << "check login content: " << content;
                                        const auto& addr = ptt.yunying().login_addr();
                                        SSDK_DLOG << "check login addr: " << addr;

                                        CURLcode res;
                                        auto *curl = curl_easy_init();
                                        ASSERT(curl);

                                        struct curl_slist *headers = nullptr;
                                        headers = curl_slist_append(headers, "Cache-Control: no-cache");
                                        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
                                        string length_str = string("Content-Length: ") + to_string(content.size());
                                        headers = curl_slist_append(headers, length_str.c_str());
                                        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                                        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
                                        curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
                                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
                                        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
                                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
                                        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
                                        curl_easy_setopt(curl, CURLOPT_STDERR, null_);
                                        bpt::ptree tree;
                                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&tree);
                                        res = curl_easy_perform(curl);
                                        if (res != CURLE_OK) {
                                                SSDK_ELOG << "curl_easy_perform() failed: " << curl_easy_strerror(res);
                                                cb(pd::INTERNAL_ERROR, string());
                                        } else {
                                                auto result = tree.get("code", "0");
                                                if (result == "10000") {
                                                        auto uid = tree.get("uid", "noname");
                                                        cb(pd::OK, uid);
                                                } else {
                                                        SSDK_DLOG << "check login failed: " << result;
                                                        cb(pd::FAILED, string());
                                                }
                                        }
                                        curl_easy_cleanup(curl);
                                });

                }
        }
}
