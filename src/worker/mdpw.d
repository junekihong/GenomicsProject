module mpd.worker.mdpw;

import std.stdio;

import mdp.common.connect;
import mdp.common.protocol;
import mdp.worker.cmd_options;
import mdp.worker.protocol;
import mdp.worker.worker;

import vibe.core.net;

struct Connections
{
    TCPConnection leader;
    TCPConnection storage;
}

void connect_to_servers(in WorkerConfiguration config, ref Connections conns)
{
    conns.leader = connect_server(config.leader, "leader");
    ubyte[4] announcement = [Announce.Worker, 0, 0, 0];
    conns.leader.write(announcement);

    conns.storage = connect_server(config.storage, "storage");
}

int main(string[] args)
{
    WorkerConfiguration config;
    bool run_program = parse_options(args, config);
    if( ! run_program )
        return 0;

    Connections conns;

    // TODO debug catches
    connect_to_servers(config, conns);

    WorkerProtocolImpl leader = new WorkerProtocolImpl(conns.leader);
    StorageProtocolImpl storage = new StorageProtocolImpl(conns.storage);
    runWorker(leader, storage);

    return 0;
}
