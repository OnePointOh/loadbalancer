#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <string>
#include <queue>
#include <netinet/in.h>

extern int SERVER_PORT;
extern int CLIENT_PORT;

struct Request {
    in_addr ip_in;   // Holds the input IP address
    in_addr ip_out;  // Holds the output IP address
    int time;         // Holds the time as a double
    
    // defaultconstructor
    Request() 
        : time(-1) {}

    // Constructor to initialize all members
    Request(in_addr& in, in_addr& out, double t) 
        : ip_in(in), ip_out(out), time(t) {}
};

struct Response {
    Request req;
    int queue_len;
    double queue_time;

    //default constructor
    Response() 
        : queue_len(0), queue_time(0){}

    // Constructor to initialize all members
    Response(Request& in, int len, double t) 
        : req(in), queue_len(len), queue_time(t) {}
};

class RequestQueue {
    public:
        // Add a request to the queue
        void addRequest(const Request& req);

        // Retrieve and remove the next request from the queue
        Request getNextRequest();

        // Check if the queue is empty
        bool isEmpty() const;


    private:
        std::queue<Request> requests; // Queue to hold Request objects
};

#endif // common_h