#ifndef TCPREQUESTCHANNEL_H
#define TCPREQUESTCHANNEL_H

#include <string>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "common.h"


class TCPRequestChannel {
public:
    // constructor for client connection
    TCPRequestChannel(std::string ip, int port);
    // constructor for server connection
    TCPRequestChannel(std::string ip, int port, int server_fd);
    
    ~TCPRequestChannel();

    // Accept an incoming connection
    TCPRequestChannel* accept_conn();

    // Read data from the socket
    void read_data(char buffer[], int n_bytes);
    // Read a Request from socket
    Request read_req();
    // Read a Response from socket
    Response read_res();

    // Write data to the socket
    void write_data(char buffer[],  int n_bytes);

    void write_data(Request req);

    void write_data(Response res);

    std::string ip_address;
    int port;
    int sock_fd;

private:
    
    
    struct sockaddr_in address;
    socklen_t address_len;

    void initialize_socket();
    void cleanup_socket();
};

#endif // TCPREQUESTCHANNEL_H
