module mdp.storage.mdps;

import std.array;
import std.stdio;

import msgpack;

import vibe.core.core;
import vibe.core.concurrency : receive, receiveOnly;

import mdp.common.cmd_options;
import mdp.common.problem;
import mdp.common.protocol;
import mdp.storage.cmd_options;
import mdp.storage.genomeOps;
import mdp.storage.solutionOps;

shared ushort myport = DEFAULT_STORAGE_PORT;
shared vibe.core.task.Task genomeTask;

/*
   Right now (Jan 17, 2014), using this function template
   causes an assertion failure in DMD.  Issue #11944
   https://d.puremagic.com/issues/show_bug.cgi?id=11944
void do_command(func, ArgT)(Task task, ArgT arg)
{
    import vibe.core.concurrency : send;
    static if( is( typeof(func(arg)) == void ) ) {
        func(arg);
    }
    else {
        auto var = func(arg);
        send(task, var);
    }
}
Use this one instead
*/
auto do_command(RetT, ArgT)(RetT function(ArgT) func)
{
    return delegate(Task task, ArgT arg) {
        import vibe.core.concurrency : send;
        static if( is( RetT == void ) ) {
            func(arg);
        }
        else {
            shared var = func(arg);
            send(task, var);
        }
    };
}

shared static this()
{
    import vibe.core.args;
    genomeTask = runTask( delegate () {
        writeln("Reading genomes...");
        initializeGenomeSystem();
        writeln("Reading solutions...");
        initializeSolutionSystem();
        
        while(true) {
            receive(
                do_command(&createGenome),
                do_command(&addGenomeData),
                do_command(&getGenomeData),
                do_command(&insertSolution),
                do_command(&queryByID),
                do_command(&queryByConditions),
                function(Task task, MessageID incomingID) {
                    writeln("Got a message id.");
                    simple_messages(task, incomingID);
                },
                function(std.variant.Variant arg) {
                    writeln("Receive fall through.");
                }
              );
        }
        //writeln("Done running genome task.");
    } );

    getOption("port", &myport, "port to listen on");
    writeln("myport = ", myport);
    auto listener = listenTCP_s(myport, &connection_handler );
}

void connection_handler(TCPConnection connection)
{
    Unpacker unpack = readBuffer(connection);
    MessageID msg_id;
    unpack.unpack(msg_id);

    switch( msg_id )
    {
        case MessageID.StoreNewGenome:
            handle_new_genome(connection, unpack);
            break;
        case MessageID.StoreNewData:
            handle_new_data(connection, unpack);
            break;
        case MessageID.StoreNewSolution:
            handle_new_solution(connection, unpack);
            break;
        case MessageID.StoreQueryByID:
            handle_query_by_id(connection, unpack);
            break;
        case MessageID.StoreQueryByCond:
            handle_query_by_cond(connection, unpack);
            break;
        case MessageID.StoreGenomeInfoQuery:
            handle_genome_info_query(connection, unpack);
            break;
        case MessageID.StoreGenomeContentQuery:
            handle_genome_content_query(connection, unpack);
            break;
        case MessageID.StoreMaxSolutionRequest:
            handle_max_solution(connection, unpack);
            break;
        case MessageID.GenomeListRequest:
            handle_genome_list(connection, unpack);
            break;
        default:
            writeln("Unknown message type: ", msg_id.stringof, " from socket ", connection);
    }

    assert(0);
}

void sendToGenome(T)(ref T msg)
    if( is( T == struct) )
{
    import vibe.core.concurrency : send;
    send(genomeTask, vibe.core.task.Task.getThis(), msg);
}

void sendToGenome(T)(T msg)
    if( is(T == enum) )
{
    import vibe.core.concurrency : send;
    send(genomeTask, vibe.core.task.Task.getThis(), msg);
}

void sendToGenome(T)(shared T * msg)
    if( is(T == struct) )
{
    import vibe.core.concurrency : send;
    send(genomeTask, vibe.core.task.Task.getThis(), msg);
}

void handle_new_genome(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling new genome.");

    CreateNewGenome msg;
    unpack.unpack(msg);

    sendToGenome(msg);

    send_ack(connection, MessageID.StoreQueryResponse);
}

void handle_new_data(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling new data");

    InsertGenomeData msg;
    unpack.unpack(msg);

    sendToGenome(msg);

    send_ack(connection, MessageID.StoreQueryResponse);
}

void handle_genome_info_query(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling genome info query.");
    
    assert(0);
}

void handle_genome_content_query(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling genome content query from socket ", connection, ".");

    QueryByName msg;
    unpack.unpack(msg);

    sendToGenome(msg);
    string resp = receiveOnly!string();

    Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
    pack.pack(MessageID.StoreGenomeContentResponse);
    pack.pack(msg.name);
    pack.pack(msg.startIndex);
    pack.pack(resp);

    sendBuffer(connection, pack.stream().data);
    connection.flush();
}

void handle_new_solution(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling new solution from socket ", connection, ".");

    shared InsertSolution msg;
    unpack.unpack(msg);
    shared InsertSolution* msg_ptr = &msg;

    sendToGenome(msg_ptr);

    send_ack(connection, MessageID.StoreQueryResponse);
}

void handle_query_by_id(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling query by id.");

    QueryByProblemID msg;
    unpack.unpack(msg);

    sendToGenome(msg);
    QueryResponse resp = receiveOnly!QueryResponse();

    netSend(connection, resp);
}

void handle_query_by_cond(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling query by conditions.");

    shared QueryByInitialConditions msg;
    unpack.unpack(msg);
    shared QueryByInitialConditions* msg_ptr = &msg;

    sendToGenome(msg_ptr);
    QueryResponse resp = receiveOnly!QueryResponse();

    netSend(connection, resp);
}

void handle_max_solution(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling next solution id request.");

    sendToGenome(MessageID.StoreMaxSolutionRequest);
    ProblemID max_id = receiveOnly!ProblemID();

    Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
    pack.pack(MessageID.StoreMaxSolutionResponse);
    pack.pack(max_id);

    sendBuffer(connection, pack.stream().data);
    connection.flush();
}

void handle_genome_list(ref TCPConnection connection, ref Unpacker unpack)
{
    debug(1) writeln("mdps.d: handling genome list request.");
    sendToGenome(MessageID.GenomeListRequest);
    GenomeInfo[string] tmp_list = receiveOnly!(GenomeInfo[string])();
    Packer!(Appender!(ubyte[])) pack = packer(appender!(ubyte[])());
    pack.pack(MessageID.GenomeListResponse);
    pack.pack(tmp_list);
    sendBuffer(connection, pack.stream().data);
    connection.flush();
}

void simple_messages(Task tid, MessageID incomingID)
{
    import vibe.core.concurrency : send;

    switch( incomingID )
    {
        case MessageID.StoreMaxSolutionRequest:
            writeln("sending max solution id.");
            shared to_send = nextSolutionID;
            send(tid, to_send);
            break;
        case MessageID.GenomeListRequest:
            // FIXME there is WAY to much copying going on here
            shared GenomeInfo[string] tmp_list;
            foreach(key, value; getGenomes() )
                tmp_list[key] = value;
            send(tid, tmp_list);
            break;
        default:
            writeln("Got an unknown message.");
            assert(0);
    }
}
