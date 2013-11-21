#include <string>

#ifndef __WORKER_CMD_OPTIONS_H__
#define __WORKER_CMD_OPTIONS_H__

struct ServerEndpoint
{
    std::string host;
    std::string port;
};

struct WorkerConfiguration
{
    ServerEndpoint leader;
    ServerEndpoint storage;
};

WorkerConfiguration parse_options(int argc, const char* argv[]);

#endif // __WORKER_CMD_OPTIONS_H__
