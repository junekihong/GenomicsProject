#include "protocol.h"

#include <stdlib.h>

WorkerActions * workerActionFactory(LeaderWorkerProtocol * w)
{
    return NULL;
}

class ClientActionImpl : public ClientActions
{
    LeaderClientProtocol * client;
    public:
    ClientActionImpl(LeaderClientProtocol * c);
    
    virtual void startGenomeUpload(const std::string& name, unsigned length);
    virtual void continueGenomeUpload(const std::vector<char>& data);
    virtual void listGenomes();
    virtual void alignmentRequest(const std::string& first, const std::string& second);
};

ClientActions * clientActionFactory(LeaderClientProtocol * c)
{
    return new ClientActionImpl(c);
}

void ClientActionImpl::startGenomeUpload(const std::string &name, unsigned int length)
{
    //TODO contact storage
}