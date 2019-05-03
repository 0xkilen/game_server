#pragma once

#include "conn.hpp"
#include "websocket/client_conn.hpp"
#include "websocket/server_conn.hpp"
#include "http/conn.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>

using namespace std;

namespace ba = boost::asio;

namespace nora {
        namespace net {

                using CONN = conn<ba::ip::tcp::socket>;
                using WS_SERVER_CONN = websocket::server_conn<ba::ip::tcp::socket>;
                using WSS_SERVER_CONN = websocket::server_conn<ba::ssl::stream<ba::ip::tcp::socket>>;
                using WS_CLIENT_CONN = websocket::client_conn<ba::ip::tcp::socket>;
                using HTTP_CONN = http::conn<ba::ip::tcp::socket>;

                using PLAYER_MGR_CONN = WS_SERVER_CONN;

        }
}
