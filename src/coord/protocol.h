#ifndef __LEADER_PROTOCOL_H__
#define __LEADER_PROTOCOL_H__

#include <vector>

#include <boost/asio/ip/tcp.hpp>

#include <msgpack.hpp>

#include "common/problem.h"
#include "common/protocol.h"
#include "common/solution.h"
#include "scheduling.h"

extern StorageProtocol* storage;

class LeaderWorkerProtocol
{
    public:
    virtual void sendProblemList(const std::vector<ProblemDescription>& problemList) = 0;
    virtual void respondToProblemClaim(bool answer) = 0;
};

class WorkerActions
{
    protected:
    LeaderWorkerProtocol * worker;
    public:
    WorkerActions(LeaderWorkerProtocol * w)
        : worker(w)
    { }
    virtual void requestProblemList() = 0;
    virtual void claimProblems(const std::vector<ProblemID>& problems) = 0;
    virtual void recieveSolution(const SolutionCertificate& solution) = 0;
};

WorkerActions * workerActionFactory(LeaderWorkerProtocol * w);

class LeaderClientProtocol
{
    public:
    virtual void sendGenomeList(const std::vector<std::string>& nameList) = 0;
    virtual void sendLocalAlignResponse(const Solution& solution) = 0;

    virtual void sendGenomeUploadResponse() = 0;
    
    virtual int getSocket() const = 0;
};

class ClientActions
{
    public:
    virtual void startGenomeUpload(const std::string& name, unsigned length) = 0;
    virtual void continueGenomeUpload(unsigned index, const std::vector<unsigned char>& data) = 0;
    virtual void finishGenomeUpload() = 0;

    virtual void listGenomes() = 0;
    virtual void alignmentRequest(const std::string& first, const std::string& second) = 0;
};

ClientActions * clientActionFactory(LeaderClientProtocol * c);

class NetworkHandler
{
    protected:
    int socket;
    
    public:
    NetworkHandler(int _socket)
        : socket(_socket)
    { }
    virtual ~NetworkHandler() { }
    
    /**
     * Returns false if the network handler should be removed and deleted.
     * The handler should not close the socket.
     */
    virtual bool handleNetwork() = 0;
    
    int getSocket() const
    {
        return socket;
    }
};

class WorkerHandler : public NetworkHandler
{
    // TODO leaks!
    WorkerActions * actions;
    
    public:
    WorkerHandler(int socket);
    
    virtual bool handleNetwork();
    
    private:
    void handleListRequest(msgpack::unpacker& unpack);
    void handleClaimRequest(msgpack::unpacker& unpack);
    void handleSolutionReport(msgpack::unpacker& unpack);
};

class ClientHandler : public NetworkHandler
{
    // TODO leaks!
    ClientActions * actions;
    
    public:
    ClientHandler(int socket);
    
    virtual bool handleNetwork();
    
    private:
    bool uploadInProgress;
    unsigned uploadProgress;
    unsigned uploadLength;
    
    void handleGenomeListRequest(msgpack::unpacker& unpack);
    void handleGenomeStart(msgpack::unpacker& unpack);
    void handleGenomeFinish();

    bool handleGenomeContinuation();
    void handleAlignmentStart(msgpack::unpacker& unpack);
};

void destroy_socket(int socket);

#endif /* __LEADER_PROTOCOL_H__ */
