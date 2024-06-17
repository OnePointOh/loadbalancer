#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "TCPRequestChannel.h"
#include "common.h"
#include <vector>
#include <chrono>


class LoadBalancer {
public:
    // Constructor
    LoadBalancer(int client_port, int server_port, int num_servers);

    // Destructor
    ~LoadBalancer();

    // accept one client connection
    void accept_client();

    // Listen for requests from clients, look for responses from servers, distribute requests
    void run();
    

private:
    TCPRequestChannel* server_mchan;
    TCPRequestChannel* client_mchan;
    std::vector<TCPRequestChannel*> server_channels;
    std::vector<TCPRequestChannel*> client_channels;
    RequestQueue requests;
    int server_port;
    int client_port;
    std::vector<int> server_fds;
    std::vector<int> client_fds;
    int server_fdmax = -1;
    int client_fdmax = -1;

    std::vector<Response> last_responses;
    std::vector<int> server_times;
    std::vector<int> additional_times;
    std::vector<std::chrono::time_point<std::chrono::system_clock>> update_times;
};

#endif // LOADBALANCER_H