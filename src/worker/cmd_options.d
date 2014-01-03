module mdp.worker.cmd_options;

import std.conv;
import std.getopt;

import mdp.common.cmd_options;

struct WorkerConfiguration
{
    ServerEndpoint leader;
    ServerEndpoint storage;
}

bool parse_options(string[] args, out WorkerConfiguration config)
{
    bool help = false;
    string leader_address = DEFAULT_SERVER ~ ":" ~ to!string(DEFAULT_PORT);
    string storage_address= DEFAULT_SERVER ~ ":" ~ to!string(DEFAULT_STORAGE_PORT);

    getopt(args,
            "help", &help,
            "server", &leader_address,
            "storage", &storage_address
          ); // TODO catch exceptions and print good messages

    if( help ) {
        import std.stdio : writeln;
        writeln("MDP Worker Options:");
        writeln("  --help                          print this help message");
        writeln("  --server arg (=", DEFAULT_SERVER, ":", DEFAULT_PORT,")  the hostname and, optionally, the port number");
        writeln("                            of the coordinator");
        writeln("  --storage arg (=", DEFAULT_SERVER, ":", DEFAULT_STORAGE_PORT, ") the hostname and, optionally, the port number");
        writeln("                            of a storage server");
        return false;
    }

    extract_server_port(leader_address, DEFAULT_PORT, config.leader);
    extract_server_port(storage_address, DEFAULT_STORAGE_PORT, config.storage);

    return true;
}
