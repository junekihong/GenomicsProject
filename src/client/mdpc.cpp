#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/asio/ip/tcp.hpp>

#include "common/cmd_options.h"
#include "common/connect.h"
#include "common/protocol.h"
#include "common/util.h"

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
    
///    try {
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
        else if( *arg_iter == "local-align" )
        {
            ++arg_iter;
            handle_local_align_args(arg_iter);
        }
        else {
            std::cout << "Unrecognized command: " << *arg_iter << "\n";
            exit(-1);
        }
//    }
//    catch( const std::exception& err )
//    {
//        std::cerr << err.what() << "\n";
//        return -1;
//    }
    return 0;
}

void connect_to_leader(tcp::iostream& leader)
{
    connect_server(leader, leaderEndpoint, "leader");
    const int announce = ANNOUNCE_CLIENT;
    sendItem(leader, announce);
    if( !leader ) {
        throw std::runtime_error("Error announcing to the leader");
    }
}

void handle_genome_args(std::vector<std::string>::iterator& arg_iter)
{
    if( *arg_iter == "upload" )
    {
        ++arg_iter;
        const std::string& filename = *arg_iter;
        if( filename.size() == 0 )
            throw std::runtime_error("You must specify a file that contains the genome");
        ++arg_iter;
        const std::string& name = *arg_iter;
        if( name.size() == 0 )
            throw std::runtime_error("You must specify a name for the genome");
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
    if( !dna_file )
        throw std::runtime_error("The file containing the genome (" + filename + ") could not be opened.");
    std::string genome = readFastaString(dna_file);
    dna_file.close();
    
    message_id_t msg_id = GENOME_UPLOAD_START_ID;
    sendItem(leader, msg_id);
    
    unsigned size = static_cast<unsigned>(name.size()); // FIXME loses precision;
    sendItem(leader, size);
    leader.write(name.data(), name.size());
    
    size = static_cast<unsigned>(genome.size()); // FIXME loses precision
    sendItem(leader, size);
    for( unsigned cur_idx = 0; cur_idx < genome.size(); cur_idx += BUFF_SIZE )
    {
        unsigned cur_chunk = std::min<unsigned>(static_cast<unsigned>(genome.size()) - cur_idx, BUFF_SIZE);
        leader.write(genome.data() + cur_idx, cur_chunk);
    }
    
    readItem(leader, msg_id);
    if(UPLOAD_REQUEST_RECIEVED_ID == msg_id)
    {
        // the upload is sucessful. We have recieved an ACK back from the leader. And it matches.
    }
}

void handle_genome_list()
{
    tcp::iostream leader;
    connect_to_leader(leader);
    
    message_id_t msg_id = GENOME_LIST_REQUEST_ID;
    sendItem(leader, msg_id);
    
    readItem(leader, msg_id);
    if( msg_id != GENOME_LIST_RESPONSE_ID )
    {
        throw std::runtime_error("Requested the genome list but got back message type " + toString(msg_id) + " instead");
    }
    
    unsigned name_count;
    readItem(leader, name_count);
    std::vector<std::string> genome_names;
    genome_names.reserve(name_count);
    for( unsigned i = 0; i < name_count; ++i )
    {
        unsigned name_len;
        readItem(leader, name_len);
        char * buffer = new char[name_len + 1]; // TODO leaks on exceptions
        leader.read(buffer, name_len);
        buffer[name_len] = 0;
        genome_names.push_back(buffer);
        delete[] buffer;
    }
    
    std::cout << "There are " << genome_names.size() << " genomes.\n";
    for( unsigned i = 0; i < genome_names.size(); ++i )
    {
        std::cout << "\t" << genome_names[i] << "\n";
    }
}

void handle_local_align_args(std::vector<std::string>::iterator& arg_iter)
{
    ++arg_iter;
    const std::string& first = *arg_iter;
    if( first.size() == 0 )
        throw std::runtime_error("You must specify genomes to align");
    ++arg_iter;
    const std::string& second = *arg_iter;
    if( second.size() == 0 )
        throw std::runtime_error("You must specify two genomes to align");
    
    tcp::iostream leader;
    connect_to_leader(leader);
    
    message_id_t msg_id = LOCAL_ALIGN_START_ID;
    sendItem(leader, msg_id);
    
    sendItem(leader, static_cast<unsigned>(first.size()));
    leader.write(first.data(), first.size());
    
    sendItem(leader, static_cast<unsigned>(second.size()));
    leader.write(second.data(), second.size());
    
    readItem(leader, msg_id);
    if( msg_id != LOCAL_ALIGN_FINISH_ID )
        throw std::runtime_error("Started");
    
    // TODO figure out what to do here.
}
