module mdp.common.cmd_options;

import std.conv;
import std.string;

enum DEFAULT_SERVER        = "localhost";
enum ushort DEFAULT_PORT          = 5528;
enum ushort DEFAULT_STORAGE_PORT  = 5529;

struct ServerEndpoint
{
    string host;
    ushort port;
};

void extract_server_port(string combined, ushort default_port, out ServerEndpoint endpoint)
{
    auto first_colon = indexOf(combined, ':');
    if( first_colon == -1 )
    {
        endpoint.host = combined;
        endpoint.port = default_port;
        return;
    }

    if( first_colon != lastIndexOf(combined, ':') ) {
        // throw error
    }

    endpoint.host = combined[0 .. first_colon];
    // TODO throws on failure
    endpoint.port = to!ushort(combined[first_colon + 1 .. $]);
}
