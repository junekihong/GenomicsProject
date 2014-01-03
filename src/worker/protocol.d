module mdp.worker.protocol;

public import mdp.common.problem;
public import mdp.common.solution;

private import std.array : Appender, appender;
private import std.conv : to;
private import msgpack;
private import mdp.common.protocol;

interface WorkerLeaderProtocol
{
    public:
    ProblemDescription[] requestProblemList();

    bool claimProblems(in ProblemID[] problems);

    void sendSolution(in SolutionCertificate solution);
}

class WorkerProtocolImpl : WorkerLeaderProtocol
{
    private:
    import vibe.core.net;

    TCPConnection socket;

    public:
    this(TCPConnection con)
    {
        socket = con;
    }

    override ProblemDescription[] requestProblemList()
    {
        // writeln("Requesting problem list");
        send_ack(socket, MessageID.ProblemListRequest);
        socket.flush();

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.ProblemListResponse )
            throw new Exception("Requested problem list, but got back message type " ~ to!string(response_msg_id) ~ " instead.");
        
        ProblemDescription[] problemList;
        unpack.unpack(problemList);
        return problemList;
    }

    override bool claimProblems(in ProblemID[] problems)
    {
        /*
         * writeln("Claiming ", problems.length, " problems");
         * writeln("They are:");
         * foreach( ProblemID id; problems )
         *     writeln("\t", id);
         */
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[]));
        pack.pack(MessageID.ProblemClaimRequest);
        pack.pack(problems);
        sendBuffer(socket, pack.stream().data);
        socket.flush();

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.ProblemClaimResponse )
            throw new Exception("Attempted to claim problems, but got back message type " ~ to!string(response_msg_id) ~ " instead.");

        bool result;
        unpack.unpack(result);
        return result;
    }

    override void sendSolution(in SolutionCertificate solution)
    {
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[]));
        pack.pack(MessageID.SolutionReport);
        pack.pack(solution);
        sendBuffer(socket, pack.stream().data);
        socket.flush();
    }
}
