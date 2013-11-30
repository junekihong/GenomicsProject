#ifndef __COMMON_CMD_OPTIONS_H__
#define __COMMON_CMD_OPTIONS_H__

#include <string>

#define DEFAULT_SERVER          "localhost"
#define DEFAULT_PORT            "5528"
#define DEFAULT_STORAGE_PORT    "5529"
#define DEFAULT_STORAGE_PORT_INT 5529

struct ServerEndpoint
{
    std::string host;
    std::string port;
};

void extract_server_port(const std::string& combined, const std::string& default_port, ServerEndpoint& endpoint);

#endif // __COMMON_CMD_OPTIONS_H__
