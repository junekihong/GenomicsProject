module mdp.client.main;

import std.array;
import std.getopt;
import std.stdio;
import std.string;

import msgpack;

import vibe.core.file;
import vibe.core.net;

import mdp.common.cmd_options;
import mdp.common.protocol;
import mdp.common.solution;

import mdp.client.fasta;

ServerEndpoint leaderEndpoint;

int main(string[] args)
{
    leaderEndpoint.host = DEFAULT_SERVER;
    leaderEndpoint.port = DEFAULT_PORT;

    args = args[1..$];
    if( args.length == 0 ) {
        writeln("Missing command.  Expected \"genome\" or \"local-align\".");
        return -1;
    }
    if( args[0] == "--server" )
    {
        if( args.length < 2 ) {
            writeln("Expected \"<server>[:<port>]\" after --server, but there are no more arguments.");
            return -1;
        }
        extract_server_port(args[1], DEFAULT_PORT, leaderEndpoint);
        args = args[2..$];
    }
    else if( args[0][0 .. 2] == "--" )
    {
        writeln("Unrecognized option: ", args[0]);
        return -1;
    }

    if( args.length == 0 ) {
        writeln("Missing command.  Expected \"genome\" or \"local-align\".");
        return -1;
    }
    switch( args[0] )
    {
        case "genome":
            handle_genome_args(args[1..$]);
            break;
        case "local-align":
            handle_local_align_args(args[1..$]);
            break;
        default:
            writeln("Unrecognized command: ", args[0]);
            break;
    }

    return 0;
}

TCPConnection connect_to_leader()
{
    TCPConnection conn = connectTCP(leaderEndpoint.host, leaderEndpoint.port);
    ubyte[4] announcement = [Announce.Client, 0, 0, 0];
    conn.write(announcement);
    return conn;
}

void handle_genome_args(string[] args)
{
    if( args[0] == "upload" )
    {
        if( args.length < 2 )
            throw new Exception("You must specify a file that contains the genome.");
        if( args.length < 3 )
            throw new Exception("You must specify a name for the genome.");
        handle_genome_upload(args[1], args[2]);
    }
    else if( args[0] == "list" ) {
        handle_genome_list();
    }
    else {
        throw new Exception(format("Unrecognized sub-command: genome %s", args[0]));
    }
}

void handle_genome_upload(string filename, string name)
{
    TCPConnection leader = connect_to_leader();

    if( !existsFile(filename) )
        throw new Exception("The file containing the genome (" ~ filename ~ ") does not exist.");
    FileInfo fileInfo = getFileInfo(filename);
    if( fileInfo.isDirectory )
        throw new Exception("The path to the genome (" ~ filename ~ ") is a directory; it must be a regular file (or symlink).");
    FileStream dna_file = openFile(filename, FileMode.read);
    const(ubyte)[] genome = readFastaString(dna_file);
    dna_file.close();

    // FIXME casting length -> uint loses precision (from ulong)
    GenomeUploadStart msg = { name, cast(uint) genome.length };
    netSend(leader, msg);
    // FIXME this flushes the stream, but that's not what we want, since we're sending more data

    leader.write(genome);
    leader.flush();

    receive_ack(leader, MessageID.UploadRequestReceived);
}

void handle_genome_list()
{
    TCPConnection leader = connect_to_leader();

    // Not really sending an ack, just a single message id
    send_ack(leader, MessageID.GenomeListRequest);
    leader.flush();

    Unpacker unpack = readBuffer(leader);

    MessageID msg_id;
    unpack.unpack(msg_id);
    if( msg_id != MessageID.GenomeListResponse )
        throw new Exception(format("Requested the genome list but got back message type %d instead.", msg_id));

    string[] genome_names;
    unpack.unpack(genome_names);

    writeln("There are ", genome_names.length, " genomes.");
    foreach( string genome; genome_names )
        writeln("\t", genome);
}

void handle_local_align_args(string[] args)
{
    if( args.length == 0 )
        throw new Exception("You must specify genomes to align.");

    while( args.length > 0 ) {
        if( args.length == 0 )
            throw new Exception("You must specify genomes to align.");
        if( args.length == 1 )
            throw new Exception("You must specify two genomes to align.");

        TCPConnection leader = connect_to_leader();

        LocalAlignStart msg = {args[0], args[1]};
        netSend(leader, msg);

        Unpacker unpack = readBuffer(leader);
        MessageID msg_id;
        unpack.unpack(msg_id);
        if( msg_id != MessageID.LocalAlignFinish )
            throw new Exception("Made an alignment request, but got a response that was not the alignment response.");

        Solution sol;
        unpack.unpack(sol);

        writeln("Maximum value ", sol.maxValue, " at location (", sol.maxValueLocation.column, ", ", sol.maxValueLocation.row, ")");
        writeln(sol.matrix);
        if( args.length > 2 )
            args = args[2 .. $];
        else
            return;
    }
}
