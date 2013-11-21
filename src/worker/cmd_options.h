#include <string>

#ifndef __WORKER_CMD_OPTIONS_H__
#define __WORKER_CMD_OPTIONS_H__

struct ServerEndpoint
{
    std::string host;
    std::string port;

    /*ServerEndpoint(const std::string& h, const std::string& p)
        : host(h), port(p)
    { }*/
};

struct WorkerConfiguration
{
    //std::string my_port; TODO Unnecessary?
    ServerEndpoint leader;
    ServerEndpoint storage;
};

WorkerConfiguration parse_options(int argc, const char* argv[]);

#endif // __WORKER_CMD_OPTIONS_H__
