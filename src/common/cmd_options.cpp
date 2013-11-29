#include <stdexcept>

#include "cmd_options.h"

void extract_server_port(const std::string& combined, const std::string& default_port, ServerEndpoint& endpoint)
{
    std::string::size_type first_colon = combined.find(":");
    if( first_colon == std::string::npos )
    {
        endpoint.host = combined;
        endpoint.port = default_port;
        return;
    }
    
    if( first_colon != combined.rfind(":") ) {
        throw std::runtime_error("Multiple ':' characters in server specification.  There may be only one, and it separates the server hostname from the port number.");
    }
    
    endpoint.host = combined.substr( 0, first_colon );
    endpoint.port = combined.substr( first_colon + 1);
}

