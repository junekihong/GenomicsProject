module mdp.storage.mdps;

import std.stdio;

import vibe.core.core;

import mdp.storage.cmd_options;
import mdp.storage.genomeOps;

void initializeSolutionSystem()
{
}

int main(string[] args)
{
    ushort myport;
    bool run_program = parse_options(args, myport);
    if( ! run_program )
        return 0;

    writeln("Reading genomes...");
    initializeGenomeSystem();
    writeln("Reading solutions...");
    initializeSolutionSystem();

    // FIXME wrapping the call to connection_handler makes the compiler happy.  Why?
    writeln("myport = ", myport);
    auto listener = listenTCP_s(myport, (conn) { connection_handler(conn); } );

    lowerPrivileges();
    runEventLoop();

    return 0;
}

void connection_handler(TCPConnection connection)
{
    assert(0);
}
