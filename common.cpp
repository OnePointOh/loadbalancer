#include "common.h"

#include <iostream>
#include <string>

int SERVER_PORT = 8080;
int CLIENT_PORT = 8081;

// Add a request to the queue
void RequestQueue::addRequest(const Request& req) {
    requests.push(req);
}

// Retrieve and remove the next request from the queue
Request RequestQueue::getNextRequest() {
    if (requests.empty()) {
        throw std::out_of_range("No requests in the queue");
    }
    Request nextRequest = requests.front();
    requests.pop();
    return nextRequest;
}

// Check if the queue is empty
bool RequestQueue::isEmpty() const {
    return requests.empty();
}