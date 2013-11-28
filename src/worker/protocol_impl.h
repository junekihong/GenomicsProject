#ifndef __WORKER_PROTOCOL_IMPL_H__
#define __WORKER_PROTOCOL_IMPL_H__

#include <boost/asio/ip/tcp.hpp>

#include "protocol.h"

class WorkerProtocolImpl : public WorkerLeaderProtocol
{
    boost::asio::ip::tcp::iostream& socket;

    public:
    WorkerProtocolImpl(boost::asio::ip::tcp::iostream& s)
        : socket(s)
    { }

    virtual void requestProblemList(std::vector<ProblemDescription>& problemList);
    virtual bool claimProblems(const std::vector<ProblemID>& problems);
    virtual void sendSolution(const SolutionCertificate& solution);
};

class StorageProtocolImpl : public StorageProtocol
{
  boost::asio::ip::tcp::iostream& socket;
  
 public:
 StorageProtocolImpl(boost::asio::ip::tcp::iostream& s)
   : socket(s)
  { }
  
  virtual bool insertSolution(const Solution& solution);
  virtual bool queryProblemID(const ProblemID& problemID);
};

#endif // __WORKER_PROTOCOL_IMPL_H__
