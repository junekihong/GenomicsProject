#ifndef __LEADER_CMD_OPTIONS_H__
#define __LEADER_CMD_OPTIONS_H__

#include <string>
#include "common/cmd_options.h"

struct LeaderConfiguration
{
    std::string myport;
    ServerEndpoint storage;
};

LeaderConfiguration parse_options(int argc, const char* argv[]);

#endif // __LEADER_CMD_OPTIONS_H__

