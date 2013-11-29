#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/asio/ip/tcp.hpp>

#include "common/cmd_options.h"
#include "common/connect.h"
#include "common/protocol.h"

#include "fasta.h"

using boost::asio::ip::tcp;

void connect_to_leader(tcp::iostream& leader);

void handle_genome_args(std::vector<std::string>::iterator& arg_iter);
void handle_genome_upload(const std::string& filename, const std::string& name);
void handle_genome_list();

void handle_local_align_args(std::vector<std::string>::iterator& arg_iter);

ServerEndpoint leaderEndpoint;
boost::asio::io_service io_service;

int main(int argc, const char* argv[])
{
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
    
    if( args.size() == 0 ) {
        std::cout << "Expected a command.\n";
        exit(-1);
    }
    
    std::vector<std::string>::iterator arg_iter = args.begin();
    
    try {
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
    }
    catch( std::exception err )
    {
        std::cerr << err.what() << "\n";
        return -1;
    }
    return 0;
}

void connect_to_leader(tcp::iostream& leader)
{
    tcp::resolver resolver(io_service);
    connect_server(leader, leaderEndpoint, resolver, "leader");
    leader << static_cast<int>(ANNOUNCE_CLIENT);
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
    tcp::iostream leader;
    connect_to_leader(leader);
    
    std::ifstream dna_file(filename.c_str());
    std::string genome = readFastaString(dna_file);
    dna_file.close();
    
    message_id_t msg_id = GENOME_UPLOAD_START_ID;
    leader << msg_id << static_cast<unsigned>(name.size());
    leader.write(name.data(), name.size());
    leader << static_cast<unsigned>(genome.size());
    for( unsigned cur_idx = 0; cur_idx < genome.size(); cur_idx += BUFF_SIZE )
    {
        unsigned cur_chunk = std::min<unsigned>(static_cast<unsigned>(genome.size()) - cur_idx, BUFF_SIZE);
        leader.write(genome.data() + cur_idx, cur_chunk);
    }
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
