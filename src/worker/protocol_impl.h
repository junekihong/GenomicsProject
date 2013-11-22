#ifndef __WORKER_PROTOCOL_IMPL_H__
#define __WORKER_PROTOCOL_IMPL_H__

#include <boost/asio/ip/tcp.hpp>

#include "protocol.h"

class WorkerProtocolImpl : public WorkerLeaderProtocol
{
    boost::asio::ip::tcp::socket& socket;

    public:
    WorkerProtocolImpl(boost::asio::ip::tcp::socket& s)
        : socket(s)
    { }

    virtual void requestProblemList(std::vector<ProblemDescription>& problemList);
    virtual bool claimProblems(const std::vector<ProblemID>& problems);
    virtual void sendSolution(const Solution& solution);
};

class StorageProtocolImpl : public StorageProtocol
{
    boost::asio::ip::tcp::socket& socket;

    public:
    StorageProtocolImpl(boost::asio::ip::tcp::socket& s)
        : socket(s)
    { }
};

#endif // __WORKER_PROTOCOL_IMPL_H__
