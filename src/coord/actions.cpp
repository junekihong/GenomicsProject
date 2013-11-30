#include "protocol.h"

#include <stdlib.h>

WorkerActions * workerActionFactory(LeaderWorkerProtocol * w)
{
    return NULL;
}

class ClientActionImpl : public ClientActions
{
    LeaderClientProtocol * client;
    int uploadCount;
    
    public:
    ClientActionImpl(LeaderClientProtocol * c);
    
    virtual void startGenomeUpload(const std::string& name, unsigned length);
    virtual void continueGenomeUpload(const std::vector<char>& data);
    virtual void finishGenomeUpload();

    virtual void listGenomes();
    virtual void alignmentRequest(const std::string& first, const std::string& second);
};

ClientActions * clientActionFactory(LeaderClientProtocol * c)
{
    return new ClientActionImpl(c);
}

//TODO
ClientActionImpl::ClientActionImpl(LeaderClientProtocol * c)
{
}

//TODO
void ClientActionImpl::startGenomeUpload(const std::string &name, unsigned int length)
{
    //TODO contact storage

    uploadCount = 0;
}

//TODO
void ClientActionImpl::continueGenomeUpload(const std::vector<char>& data)
{   
    //TODO contact storage
}

void ClientActionImpl::finishGenomeUpload()
{
    client->sendGenomeUploadResponse();
}

//TODO
void ClientActionImpl::listGenomes()
{

}

//TODO
void ClientActionImpl::alignmentRequest(const std::string& first, const std::string& second)
{
    
    //client->sendLocalAlignResponse();
}

