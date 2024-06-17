#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "TCPRequestChannel.h"
#include "common.h"

class WebServer {
    public:
        // Constructor
        WebServer(const std::string& ip, int port);

        // Destructor
        ~WebServer();

        // Run the server
        void run();

        void addRequest(Request req);


    private:
        TCPRequestChannel* channel;
        RequestQueue* requestQueue;
        int total_time;
        int queue_len;
    };

#endif // WEBSERVER_H
