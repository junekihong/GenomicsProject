#ifndef __WORKER_H__
#define __WORKER_H__

#include "protocol.h"

void runWorker(WorkerLeaderProtocol& leader, StorageProtocol& storage);

#endif // __WORKER_H__
