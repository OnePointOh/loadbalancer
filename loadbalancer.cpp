#include "loadbalancer.h"
#include "common.h"
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <queue>
#include <utility>
#include <chrono>

LoadBalancer::LoadBalancer(int p, int p2, int num_servers) {
    client_port = p;
    server_port = p2;
    std::cout << "LoadBalancer server_port" << std::endl;
    server_mchan = new TCPRequestChannel("", server_port);
    std::cout << "LoadBalancer client_port" << std::endl;
    client_mchan = new TCPRequestChannel("", client_port);
    std::cout << "LoadBalancer waiting for server connections" << std::endl;
    auto now = std::chrono::system_clock::now();
    for (int i = 0; i < num_servers; i++) {
        TCPRequestChannel* server_channel = server_mchan->accept_conn();
        std::cout << "Server " << i << "connected" << std::endl;
        server_channels.push_back(server_channel);
        server_fds.push_back(server_channel->sock_fd);
        server_fdmax = server_channel->sock_fd;
        last_responses.push_back(Response());
        server_times.push_back(0);
        update_times.push_back(now);
        additional_times.push_back(0);
    }
    std::cout << "Servers connected" << std::endl;
}

LoadBalancer::~LoadBalancer() {
    delete server_mchan;
    delete client_mchan;
    for (auto* channel : server_channels) {
        delete channel;
    }
    for (auto* channel : client_channels) {
        delete channel;
    }
}

void LoadBalancer::accept_client() {
    TCPRequestChannel* client_channel = client_mchan->accept_conn();
    client_channels.push_back(client_channel);
    client_fds.push_back(client_channel->sock_fd);
    client_fdmax = client_channel->sock_fd;
    std::cout << "LB accepted request client" << std::endl;
}

void LoadBalancer::run() {

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int cycles = 0;
    std::cout << "LB Running" << std::endl;
    bool running = true;

    while (running) {
    
    //cycles += 1;

    // Check for client requests
    fd_set read_fds;
    FD_ZERO(&read_fds);
    for (int sock_fd : client_fds){
        FD_SET(sock_fd, &read_fds);
    }
    
    if (select(client_fdmax+1, &read_fds, NULL, NULL, &tv) > 0) {
        for (TCPRequestChannel* chan : client_channels){
            if (FD_ISSET(chan->sock_fd, &read_fds)){
                // NOTE: channel may block if it has byte available
                // but not full req recieved yet
                Request req = chan->read_req();
                char from[INET_ADDRSTRLEN];
                char to[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(req.ip_in), from, INET_ADDRSTRLEN);
                inet_ntop(AF_INET, &(req.ip_out), to, INET_ADDRSTRLEN);
                std::cout << "LB recieved  IP_IN=" << from << "  IP_OUT=" << to << std::endl;
                
                // if kill message, break
                if (strcmp(from, "0.0.0.0") == 0){
                    running = false;
                    break;
                }

                requests.addRequest(req);
            }
        }
    }

    auto now = std::chrono::system_clock::now();

    // Check for server responses and update all server times
    std::priority_queue<std::pair<int, int> > pq;
    FD_ZERO(&read_fds);
    for (int sock_fd : server_fds){
        FD_SET(sock_fd, &read_fds);
    }
    if (select(server_fdmax+1, &read_fds, NULL, NULL, &tv) > 0) {
        for (u_int i = 0; i < server_channels.size(); i++){
            TCPRequestChannel* chan = server_channels[i];
            
            if (FD_ISSET(server_fds[i], &read_fds)){
                // NOTE: channel may block if it has byte available
                // but not full res recieved yet
                Response res = chan->read_res();
                std::cout << "Server " << i << " finished Req. Time left: " << res.queue_time
                    << "ms. Reqs left: " << res.queue_len << std::endl; 

                last_responses[i] = res;
                additional_times[i] = 0;
                server_times[i] = res.queue_time;
                update_times[i] = now;
            }
            /*
            else{ // if no server response yet, update estimated time
                double elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - update_times[i]).count();
                server_times[i] = last_responses[i].queue_time + additional_times[i] - elapsed_time;
            }
            */
        }
    }

    // update pq
    for (u_int i = 0; i < server_times.size(); i++){
        pq.push(std::make_pair(-server_times[i], i));
    }

    // Distribute requests
    
    while(!requests.isEmpty() && !pq.empty()){
        Request next_req = requests.getNextRequest();
        std::pair<int, int> next_pair = pq.top();
        pq.pop();

        int i = next_pair.second;

        std::cout << "time: " << next_pair.first << std::endl;
        char from[INET_ADDRSTRLEN];
        char to[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(next_req.ip_in), from, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(next_req.ip_out), to, INET_ADDRSTRLEN);
        std::cout << "sending request to Server " << i << "  IP_IN=" << from << "  IP_OUT="
            << to << "  TIME=" << next_req.time << std::endl;

        server_channels[i]->write_data(next_req);
        server_times[i] += next_req.time;
        pq.push(std::make_pair(server_times[i], i));
    }
    

    } // end while loop

    std::cout << "cycles: " << cycles << std::endl;

    struct sockaddr_in sa;
    inet_pton(AF_INET, "0.0.0.0", &(sa.sin_addr));
    for (TCPRequestChannel* chan : server_channels){
        

        Request kill_req = Request(sa.sin_addr, sa.sin_addr, 0);
        chan->write_data(kill_req);
    }
}

/*
int main(){
    LoadBalancer * lb = new LoadBalancer(CLIENT_PORT, SERVER_PORT, 5);
    lb->accept_client();
    lb->run();
    std::cout << "LB Sent all kill messages";
    delete lb;

}
*/