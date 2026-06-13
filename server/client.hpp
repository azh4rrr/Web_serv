#pragma once
#include "../config/Config.hpp"
#include "../http/request.hpp"
#include "../http/response.hpp"
#include <poll.h>
// #include <bool.h>



class Client : public Request, public Response {
private:

    int _fd;
    Config _config;
    Request _request;
    Response _response;
    // std::string _readBuffer;
    // bool _headerComplete;
    
public:
    Client(int fd, const Config &config);
    int getFd() const;
    const Config &getConfig() const;
    // bool isRequestComplete() const ;
    // bool isheaderComplete() const ;
    // void appendToBuffer(const char* data, size_t len) {
    //     _readBuffer.append(data, len);
    // }
    void readRequest();
    void setResponse(const std::string& response) {
        _response.setRawResponse(response);
    }
    const Response& getResponse() const {
        return _response;
    }
    // void displayrequest();
    // void parseRequest();
    // Request& getrequest() {return _request;}
    // void readytosend(const char* data, size_t len);

};