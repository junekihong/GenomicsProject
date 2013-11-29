#ifndef __COMMON_CONNECT_H__
#define __COMMON_CONNECT_H__

#include <boost/asio/ip/tcp.hpp>

#include "cmd_options.h"

void connect_server(boost::asio::ip::tcp::iostream& socket, const ServerEndpoint& server, boost::asio::ip::tcp::resolver& resolver, const std::string& name);

#endif // __COMMON_CONNECT_H__
