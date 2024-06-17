#include "TCPRequestChannel.h"
#include "common.h"

TCPRequestChannel::TCPRequestChannel(std::string ip, int port) 
    : ip_address(ip), port(port), sock_fd(-1){
    initialize_socket();
}

TCPRequestChannel::TCPRequestChannel(std::string ip, int port, int sock_fd) 
    : ip_address(ip), port(port), sock_fd(sock_fd) {
    initialize_socket();
}

TCPRequestChannel::~TCPRequestChannel() {
    cleanup_socket();
}

void TCPRequestChannel::initialize_socket() {
    // if socket is server
    if (ip_address == ""){
            // if server-client
            if (sock_fd > 0){
                address.sin_family = AF_INET;
                address.sin_addr.s_addr = INADDR_ANY;
                address.sin_port = htons(port);
                address_len = sizeof(address);
                return;
            }

            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port);
            address_len = sizeof(address);

            int opt = 1;

            if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                throw std::runtime_error("Socket failed");
            }

            if (setsockopt(sock_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)) == -1) {
                   throw std::runtime_error("setsockopt failed");
            }

            if (bind(sock_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                throw std::runtime_error("Bind failed");
            }
            if (listen(sock_fd, 3) < 0) {
                throw std::runtime_error("Listen failed");
            }
        }
    // if client
    else{
        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                throw std::runtime_error("Socket failed");
            }
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address_len = sizeof(address);  

        if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr)
        <= 0) {
            throw std::runtime_error("Bad address");
        }

        int status;
        if ((status = connect(sock_fd, (struct sockaddr*)&address,sizeof(address)))< 0) {
            throw std::runtime_error("Connection failed STATUS=" + std::to_string(status));
        }
    }
}

void TCPRequestChannel::cleanup_socket() {
    if (sock_fd != -1) {
        close(sock_fd);
    }
}

TCPRequestChannel* TCPRequestChannel::accept_conn() {
    int new_sock_fd = accept(sock_fd, (struct sockaddr*)&address, &address_len);
    if (new_sock_fd < 0) {
        throw std::runtime_error("Accept failed");
    }
    TCPRequestChannel* new_chan = new TCPRequestChannel("", port, new_sock_fd);
    return new_chan;
}

void TCPRequestChannel::read_data(char buffer[], int n_bytes) {
    int bytes_read = recv(sock_fd, buffer, n_bytes, 0);
    if (bytes_read < 0) {
        throw std::runtime_error("Read failed");
    }
}

Request TCPRequestChannel::read_req() {
    Request new_req = Request();
    int size = sizeof(Request);
    this->read_data((char*)&new_req, size);
    return new_req;
}

Response TCPRequestChannel::read_res() {
    Response new_res = Response();
    int size = sizeof(Response);
    this->read_data((char*)&new_res, size);
    return new_res;
}


void TCPRequestChannel::write_data(char buffer[],  int n_bytes) {
    int bytes_sent = send(sock_fd, buffer, n_bytes, 0);
    if (bytes_sent < 0) {
        throw std::runtime_error("Write failed");
    }
}

void TCPRequestChannel::write_data(Request req) {
    this->write_data((char*)&req, sizeof(req));
}

void TCPRequestChannel::write_data(Response res) {
    this->write_data((char*)&res, sizeof(res));
}