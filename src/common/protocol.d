module mdp.common.protocol;

import std.array;
import std.stdio;
import std.string;

import msgpack;

import vibe.core.net;
import vibe.core.stream;

import mdp.common.location;
import mdp.common.problem;
import mdp.common.solution;

enum Announce : ubyte
{
    Client = 1,
    Leader = 2,
    Worker = 3,
    Storage = 4,
}

enum MessageID : int
{
    ProblemListRequest          =  5,
    ProblemListResponse         =  5,

    ProblemClaimRequest         =  6,
    ProblemClaimResponse        =  6,

    SolutionReport              =  7,

    GenomeUploadStart           =  8,
    UploadRequestReceived       =  8,

    GenomeListRequest           =  9,
    GenomeListResponse          =  9,

    LocalAlignStart             = 10,
    LocalAlignFinish            = 11,

    StoreNewGenome              = 12,
    StoreNewData                = 13,
    StoreNewSolution            = 14,
    StoreQueryByID              = 15,
    StoreQueryByCond            = 16,

    StoreGenomeInfoQuery        = 18,
    StoreGenomeInfoResponse     = 18,

    StoreGenomeContentQuery     = 19,
    StoreGenomeContentResponse  = 19,

    StoreQueryResponse          = 20,

    StoreMaxSolutionRequest     = 21,
    StoreMaxSolutionResponse    = 21,
}

void sendBuffer(OutputStream strm, const(ubyte)[] buff)
{
    uint length = cast(uint)buff.length; // FIXME loses precision
    static assert(uint.sizeof == 4); // ensures binary compatability with C++
    ubyte[4] lenBuf;
    foreach( int idx ; 0 .. 4) {
        lenBuf[idx] = (length >> (8 * idx)) & 0xFF;
    }
    strm.write(lenBuf);
    strm.write(buff);
}

Unpacker readBuffer(InputStream strm)
{
    ubyte[4] lenBuf;
    strm.read(lenBuf);
    uint length = 0;
    static assert(uint.sizeof == 4); // ensures binary compatability with C++
    foreach( int idx ; 0 .. 4 ) {
        length += lenBuf[idx] << (8 * idx);
    }
    debug(1)
        writeln("Reading buffer of ", length, " bytes");

    ubyte[] fullBuffer = new ubyte[length];
    strm.read(fullBuffer);
    debug(1) {
        //writeln("Read buffer:\n", fullBuffer);
        foreach( ubyte b; fullBuffer )
            writef(" %02X", b);
        write("\n");
    }
    return Unpacker(fullBuffer, fullBuffer.length);
}

void receive_ack(InputStream sock, MessageID expected)
{
    Unpacker unpack = readBuffer(sock);

    MessageID code;
    unpack.unpack(code);

    if( code != expected )
        throw new Exception(format("Expected ack %d, but got another one: %d.", expected, code));
}

void send_ack(OutputStream sock, MessageID code)
{
    Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[]));
    pack.pack(code);
    sendBuffer(sock, pack.stream().data);
}

struct GenomeInfo
{
    string name;
    uint length;
}

class QueryResponse
{
    public:
    // True if a match was found
    bool success;
    // True if an exact match was found
    bool exactMatch;

    // Gvien problem description.
    ProblemDescription problemDescription;

    // Solution description. Maximum value and location in the matrix.
    int maxValue; // TODO change to nullable?
    Location location;

    // The solution.
    // This may be omitted.
    Solution * sol;

    this()
    {
        success = false;
        exactMatch = false;
        maxValue = -1;
        sol = null;
    }

    // Deep copy
    this(QueryResponse other)
    {
        success = other.success;
        exactMatch = other.exactMatch;
      	problemDescription = other.problemDescription.dup;
		maxValue = other.maxValue;
		location = other.location;
		
		if( other.sol ) {
			sol = new Solution;
			(*sol) = *(other.sol);
		}
        else {
            sol = null;
        }
    }

    void toMsgpack(Packer)(ref Packer packer) const
    {
        if( sol is null )
            packer.beginArray(6);
        else
            packer.beginArray(7);
        packer.pack(success);
        packer.pack(exactMatch);
        packer.pack(problemDescription);
        packer.pack(maxValue);
        packer.pack(location);
        packer.pack( sol !is null );
        if( sol !is null )
            packer.pack(*sol);
    }

    void fromMsgpack(ref Unpacker unpacker)
    {
        bool hasSolution;
        auto length = unpacker.beginArray();
        if( length < 6 )
            throw new Exception("Error unpacking QueryResponse");

        unpacker.unpack(success);
        unpacker.unpack(exactMatch);
        unpacker.unpack(problemDescription);
        unpacker.unpack(maxValue);
        unpacker.unpack(location);
        unpacker.unpack(hasSolution);
        if( 6 + (hasSolution ? 1 : 0) != length )
            throw new Exception("Wrong number of elements in QueryResponse");
        if( hasSolution )
        {
            sol = new Solution();
            unpacker.unpack(*sol);
        }
    }
}

struct GenomeUploadStart
{
    static enum id = MessageID.GenomeUploadStart;
    string name;
    uint length;
}

struct LocalAlignStart
{
    static enum id = MessageID.LocalAlignStart;
    string genome_1;
    string genome_2;
}

// Move into leader submodule?
struct ClaimProblems
{
    static enum id = MessageID.ProblemClaimRequest;
    ProblemID[] problems;
}

struct SendSolutionReport
{
    static enum id = MessageID.SolutionReport;
    SolutionCertificate solution;
}

interface WorkerLeaderProtocol
{
    public:
    ProblemDescription[] requestProblemList();

    bool claimProblems(in ClaimProblems msg);

    void sendSolution(in SendSolutionReport msg);
}

// TODO move into storage submodule?
struct CreateNewGenome
{
    static enum id = MessageID.StoreNewGenome;
    string name;
    uint length;
}

struct InsertGenomeData
{
    static enum id = MessageID.StoreNewData;
    string name;
    uint index; // TODO move this after data
    string data;
}

struct InsertSolution
{
    static enum id = MessageID.StoreNewSolution;
    ProblemDescription prob;
    Solution solution;
}

struct QueryByProblemID
{
    static enum id = MessageID.StoreQueryByID;
    ProblemID prob;
    bool entireSolution;
}

struct QueryByInitialConditions
{
    static enum id = MessageID.StoreQueryByCond;
    ProblemDescription problemDescription;
    bool wantPartials;
}

struct QueryByName
{
    static enum id = MessageID.StoreGenomeContentQuery;
    string name;
    int startIndex;
    int length;
}

// TODO add constraints to the MessageType
void sendMessage(Packer, MessageType)(ref Packer packer, in MessageType message)
{
    packer.pack(MessageType.id);
    foreach(elem; message.tupleof)
        packer.pack(elem);
}

final void netSend(MessageType)(TCPConnection socket, in MessageType msg)
{
    Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
    sendMessage(pack, msg);
    sendBuffer(socket, pack.stream().data);
    socket.flush();
}

interface StorageProtocol
{
    public:

    void createNewGenome(in CreateNewGenome msg);
    void insertGenomeData(in InsertGenomeData msg);
    bool insertSolution(in InsertSolution msg);
    QueryResponse queryByProblemID(in QueryByProblemID msg);
    QueryResponse queryByInitialConditions(in QueryByInitialConditions msg);

    string queryByName(in QueryByName msg);

    ProblemID getNextSolutionID();
    void getGenomeList(ref int[string] genome_list);
}

class StorageProtocolImpl : StorageProtocol
{
    private:
    TCPConnection socket;


    public:
    this(TCPConnection con)
    {
        socket = con;
    }

    override void createNewGenome(in CreateNewGenome msg)
    {
        netSend(socket, msg);
        receive_ack(socket, MessageID.StoreQueryResponse);
    }

    override void insertGenomeData(in InsertGenomeData msg)
    {
        netSend(socket, msg);
        receive_ack(socket, MessageID.StoreQueryResponse);
    }

    // TODO should this really return bool?
    override bool insertSolution(in InsertSolution msg)
    {
        netSend(socket, msg);
        receive_ack(socket, MessageID.StoreQueryResponse);
        return true;
    }

    override QueryResponse queryByProblemID(in QueryByProblemID msg)
    {
        netSend(socket, msg);

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.StoreQueryResponse )
            throw new Exception("Error. Storage did not properly respond to our query by problem id.");

        QueryResponse response = new QueryResponse();
        unpack.unpack(response);
        return response;
    }

    override QueryResponse queryByInitialConditions(in QueryByInitialConditions msg)
    {
        netSend(socket, msg);

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.StoreQueryResponse )
            throw new Exception("Error. Storage did not properly respond to our query by problem initial conditions.");

        QueryResponse response = new QueryResponse();
        unpack.unpack(response);
        return response;
    }

    override string queryByName(in QueryByName msg)
    {
        netSend(socket, msg);

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.StoreQueryResponse )
            throw new Exception("Error. Storage did not properly respond to our query by name.");

        string responseName;
        int responseStartIndex;
        string genome;

        unpack.unpack(responseName);
        unpack.unpack(responseStartIndex);
        unpack.unpack(genome);

        return genome;
    }

    override ProblemID getNextSolutionID()
    {
        // TODO rename send_ack
        send_ack(socket, MessageID.StoreMaxSolutionRequest);
        socket.flush();

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.StoreMaxSolutionResponse )
        {
            import std.conv : to;
            throw new Exception("Storage responded to max solution ID request with message type " ~ to!string(response_msg_id));
        }

        ProblemID id;
        unpack.unpack(id);
        return id;
    }

    override void getGenomeList(ref int[string] genome_list)
    {
        send_ack(socket, MessageID.GenomeListRequest);
        socket.flush();

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.GenomeListResponse )
        {
            import std.conv : to;
            throw new Exception("Storage responded to genome list request with message type " ~ to!string(response_msg_id));
        }

        GenomeInfo[string] tmp_list;
        unpack.unpack(tmp_list);
        foreach( name, info; tmp_list )
            genome_list[name] = info.length;
    }
}
