#include "common.h"
#include "loadbalancer.h"
#include "webserver.h"
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <thread>
#include <unistd.h>
#include <random>
#include <chrono>


struct in_addr generateRandomIP() {
    in_addr addr;
    uint32_t ip = 0;

    // Generate each byte of the IP address
    uint8_t byte1 = rand() % 255 + 1;
    uint8_t byte2 = rand() % 256;
    uint8_t byte3 = rand() % 256;
    uint8_t byte4 = rand() % 256;

    ip = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
    addr.s_addr = htonl(ip);

    return addr;
}

double generateRandomTime(double min, double max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<> distrib(min, max);

    int random_number = distrib(gen);

    return random_number;
}

Request createRandomRequest() {
    in_addr ip_in = generateRandomIP();
    in_addr ip_out = generateRandomIP();
    int time = generateRandomTime(1000, 10000);

    return Request(ip_in, ip_out, time);
}

void fillRequestQueue(int num, RequestQueue& q) {
    for (int i = 0; i < num; ++i) {
        Request req = createRandomRequest();
        q.addRequest(req);
    }
}

void runServerInThread(int i) {
    std::cout << "Starting Server " << i << std::endl;
    WebServer * server = new WebServer("127.0.0.1", SERVER_PORT);
    server->run();
    std::cout << "Closed Server " << i << std::endl;
    delete server;
}

void runLoadBalancerInThread(int num) {
    LoadBalancer * lb = new LoadBalancer(CLIENT_PORT, SERVER_PORT, num);
    lb->accept_client();
    lb->run();
    std::cout << "LB Sent all kill messages";
    delete lb;
}

int main(){
    int numWebServers, numRequests, requestSpeed;
    std::cout << "Enter the number of web servers to start: ";
    std::cin >> numWebServers;

    std::cout << "Enter the number of requests to simulate: ";
    std::cin >> numRequests;

    std::cout << "Enter how fast requests should arrive (1-10): ";
    std::cin >> requestSpeed;

    RequestQueue rq = RequestQueue();
    fillRequestQueue(numRequests, rq);

    std::vector<WebServer *> servers;
    std::vector<std::thread> threads;
    std::thread lb_thread(runLoadBalancerInThread, numWebServers);
    // wait for server to startup

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    TCPRequestChannel* req_chan = new TCPRequestChannel("127.0.0.1", CLIENT_PORT);
    for (int i = 0; i < numWebServers % 20; i++){
        std::thread new_thread(runServerInThread, i);
        threads.push_back(std::move(new_thread));
    }
    std::cout << "client sending requests" << std::endl;
    while (!rq.isEmpty()){
        Request new_req = rq.getNextRequest();
        char from[INET_ADDRSTRLEN];
        char to[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(new_req.ip_in), from, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(new_req.ip_out), to, INET_ADDRSTRLEN);
        std::cout << "sending request to LB  IP_IN=" << from << "  IP_OUT="
            << to << "  TIME=" << new_req.time << std::endl;
        req_chan->write_data(new_req);
        int wait_time = generateRandomTime(500, 5000) / requestSpeed;
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
    }
    // wait 10 sec, then send kill message
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    struct sockaddr_in sa;
    inet_pton(AF_INET, "0.0.0.0", &(sa.sin_addr));
    Request kill_req = Request(sa.sin_addr, sa.sin_addr, 0);
    req_chan->write_data(kill_req);

    for (u_int i = 0; i < threads.size(); i++){
        threads[i].join();
    }

    lb_thread.join();

    delete req_chan;
}