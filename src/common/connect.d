module mdp.common.connect;

import std.stdio;

public import vibe.core.net : TCPConnection;

import mdp.common.cmd_options;

TCPConnection connect_server(in ServerEndpoint server, string name)
{
    try {
        import vibe.core.net : connectTCP;
        writeln("Attempting to connect to ", name, " at ", server.host, ":", server.port, "...");
        return connectTCP(server.host, server.port);
    }
    catch(Exception e)
    {
        writeln("Connecting to ", name, " at ", server.host, ":", server.port, " failed: ", e.msg);
        writeln("Aborting");
        throw e;
    }
}
