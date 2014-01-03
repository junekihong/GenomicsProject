module mdp.common.protocol;

import std.array;
import std.stdio;
import std.string;

import msgpack;

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


interface StorageProtocol
{
    public:

    void createNewGenome(string name, uint length);
    void insertGenomeData(string name, out uint index, string data);
    bool insertSolution(ProblemDescription prob, in Solution solution);
    QueryResponse queryByProblemID(in ProblemID prob, bool entireSolution);
    QueryResponse queryByInitialConditions(in ProblemDescription problemDescription, bool wantPartials);

    string queryByName(string name, int startIndex, int length);

    ProblemID getNextSolutionID();
    void getGenomeList(ref int[string] genome_list);
}

class StorageProtocolImpl : StorageProtocol
{
    private:
    import vibe.core.net;
    
    TCPConnection socket;


    public:
    this(TCPConnection con)
    {
        socket = con;
    }

    override void createNewGenome(string name, uint length)
    {
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
        pack.pack(MessageID.StoreNewGenome);
        pack.pack(name);
        pack.pack(length);
        sendBuffer(socket, pack.stream().data);
        socket.flush();

        receive_ack(socket, MessageID.StoreQueryResponse);
    }

    override void insertGenomeData(string name, out uint index, string data)
    {
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
        pack.pack(MessageID.StoreNewData);
        pack.pack(name);
        pack.pack(index);
        pack.pack(data);
        sendBuffer(socket, pack.stream().data);
        socket.flush();

        receive_ack(socket, MessageID.StoreQueryResponse);
    }

    // TODO should this really return bool?
    override bool insertSolution(ProblemDescription prob, in Solution solution)
    {
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
        pack.pack(MessageID.StoreNewSolution);
        //pack.pack(prob);
        prob.toMsgpack(pack);
        pack.pack(solution);
        sendBuffer(socket, pack.stream().data);
        socket.flush();

        receive_ack(socket, MessageID.StoreQueryResponse);
        return true;
    }

    override QueryResponse queryByProblemID(in ProblemID prob, bool entireSolution)
    {
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
        pack.pack(MessageID.StoreQueryByID);
        pack.pack(prob);
        pack.pack(entireSolution);
        sendBuffer(socket, pack.stream().data);
        socket.flush();

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.StoreQueryResponse )
            throw new Exception("Error. Storage did not properly respond to our query by problem id.");

        QueryResponse response = new QueryResponse();
        unpack.unpack(response);
        return response;
    }

    override QueryResponse queryByInitialConditions(in ProblemDescription problemDescription, bool wantPartials)
    {
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
        pack.pack(MessageID.StoreQueryByCond);
        pack.pack(problemDescription);
        pack.pack(wantPartials);
        sendBuffer(socket, pack.stream().data);
        socket.flush();

        Unpacker unpack = readBuffer(socket);
        MessageID response_msg_id;
        unpack.unpack(response_msg_id);
        if( response_msg_id != MessageID.StoreQueryResponse )
            throw new Exception("Error. Storage did not properly respond to our query by problem initial conditions.");

        QueryResponse response = new QueryResponse();
        unpack.unpack(response);
        return response;
    }

    override string queryByName(string name, int startIndex, int length)
    {
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
        pack.pack(MessageID.StoreQueryByCond);
        pack.pack(name);
        pack.pack(startIndex);
        pack.pack(length);
        sendBuffer(socket, pack.stream().data);
        socket.flush();

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
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
        pack.pack(MessageID.StoreMaxSolutionRequest);
        sendBuffer(socket, pack.stream().data);
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
        Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
        pack.pack(MessageID.GenomeListRequest);
        sendBuffer(socket, pack.stream().data);
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
