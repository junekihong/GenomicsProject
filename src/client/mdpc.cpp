#include <iostream>
#include <string>
#include <vector>

#include "common/cmd_options.h"

void handle_genome_args(std::vector<std::string>::iterator& arg_iter);
void handle_genome_upload(const std::string& filename, const std::string& name);
void handle_genome_list();

void handle_local_align_args(std::vector<std::string>::iterator& arg_iter);

int main(int argc, const char* argv[])
{
    ServerEndpoint leaderEndpoint;
    leaderEndpoint.host = DEFAULT_SERVER;
    leaderEndpoint.port = DEFAULT_PORT;
    /* convert the arguments to strings.
     * but skip the first one, since we don't care */
    std::vector<std::string> args;
    args.reserve(argc-1);
    for( int i = 1; i < argc; ++i )
    {
        args.push_back(argv[i]);
    }
    
    std::vector<std::string>::iterator arg_iter = args.begin();
    
    if( *arg_iter == "--server" )
    {
        arg_iter++;
        extract_server_port(*arg_iter, DEFAULT_PORT, leaderEndpoint);
        arg_iter++;
    }
    else if( arg_iter->substr(0, 2) == "--" )
    {
        std::cout << "Unrecognized option: " << *arg_iter << "\n";
        exit(-1);
    }
    
    if( *arg_iter == "genome" )
    {
        ++arg_iter;
        handle_genome_args(arg_iter);
    }
    else if( *arg_iter == "local_align" )
    {
        ++arg_iter;
        handle_local_align_args(arg_iter);
    }
    else {
        std::cout << "Unrecognized command: " << *arg_iter << "\n";
        exit(-1);
    }
    return 0;
}

void handle_genome_args(std::vector<std::string>::iterator& arg_iter)
{
    if( *arg_iter == "upload" )
    {
        ++arg_iter;
        const std::string& filename = *arg_iter;
        ++arg_iter;
        const std::string& name = *arg_iter;
        handle_genome_upload(filename, name);
    }
    else if( *arg_iter == "list" )
    {
        handle_genome_list();
    }
    else
    {
        std::cout << "Unrecognized sub-command: genome " << *arg_iter << "\n";
        exit(-1);
    }
}

void handle_genome_upload(const std::string& filename, const std::string& name)
{
    
}

void handle_genome_list()
{
    
}

void handle_local_align_args(std::vector<std::string>::iterator& arg_iter)
{
    ++arg_iter;
    const std::string& first = *arg_iter;
    ++arg_iter;
    const std::string& second = *arg_iter;
}
