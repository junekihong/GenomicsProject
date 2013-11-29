#ifndef __WORKER_CMD_OPTIONS_H__
#define __WORKER_CMD_OPTIONS_H__

#include <string>
#include "common/cmd_options.h"

struct WorkerConfiguration
{
    ServerEndpoint leader;
    ServerEndpoint storage;
};

WorkerConfiguration parse_options(int argc, const char* argv[]);

#endif // __WORKER_CMD_OPTIONS_H__
