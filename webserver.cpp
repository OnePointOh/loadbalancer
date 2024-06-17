#include "webserver.h"
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>

WebServer::WebServer(const std::string& ip, int port) {
    channel = new TCPRequestChannel(ip, port);
    requestQueue = new RequestQueue();
}

WebServer::~WebServer() {
    delete channel;
    delete requestQueue;
}

void WebServer::addRequest(Request req) {
    if (req.time > 0){
        total_time += req.time;
    }
    
    queue_len += 1;
    requestQueue->addRequest(req);
}


void WebServer::run() {
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(channel->sock_fd, &rfds);
    bool running = true;

    while (running) {

        

        // check for requests being sent here
        while (select(channel->sock_fd+1, &rfds, NULL, NULL, 0) > 0){
            Request req = channel->read_req();
            // check if req is kill request
            char from[INET_ADDRSTRLEN];
            char to[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(req.ip_in), from, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(req.ip_out), to, INET_ADDRSTRLEN);
            
            
            if (strcmp(from, "0.0.0.0") == 0){
                running = false;
                break;
            }
            std::cout << "Server recieved  IP_IN=" << from << "  IP_OUT=" << to << std::endl;
            
            this->addRequest(req);

            FD_ZERO(&rfds);
            FD_SET(channel->sock_fd, &rfds);
        }

        if (!running){
            break;
        }


        // process next request
        try {
            if (this->requestQueue->isEmpty()){
                continue;
            }
            Request req = this->requestQueue->getNextRequest();
            int time = req.time;

            // Wait for req.time to simulate doing a request
            std::this_thread::sleep_for(std::chrono::milliseconds(time));
            total_time -= time;
            queue_len -= 1;

            Response new_response = Response(req, queue_len, total_time);
            channel->write_data(new_response);

            

        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }

        
    }
}

/*
int main(){
    std::cout << "Starting Server" << std::endl;
    WebServer * server = new WebServer("127.0.0.1", SERVER_PORT);
    server->run();
    std::cout << "Closed Server" << std::endl;
    delete server;
}
*/
