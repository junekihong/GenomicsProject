module mdp.storage.cmd_options;

import std.getopt;
import std.stdio;

import mdp.common.cmd_options;

bool parse_options(string[] args, out ushort myport)
{
    bool help = false;
    myport = DEFAULT_STORAGE_PORT;

    getopt(args,
            "help", &help,
            "port", &myport
         ); // TODO catch exceptions and print good messages

    if( help )
    {
        writeln("MDP Storage Options:");
        writeln("  --help                print this help message");
        writeln("  --port arg (=", DEFAULT_STORAGE_PORT, ")    the port number of this storage node should listen on");
        return false;
    }

    return true;
}
