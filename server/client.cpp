#include "client.hpp"


// bool Client::isheaderComplete() const
// {
//     return _readBuffer.find("\r\n\r\n") != std::string::npos;
// }
#include <iostream>
#include <string>
void Client::processResponse()
{
    // _response.setConfig(_config);
    // _response.setRequest(*this);
    // _response.responseprocces();
    std::cout << "method: " << getMethod() << std::endl;
    if (Request::getMethod() == GET)
    {
        // _response.handleGet();
        handelGet();
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        setResponse(response);
    }
    else if( Request::getMethod() == POST)
    {
        // _response.handlePost();
        handelPost();
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        setResponse(response);
    }
    else if(Request::getMethod() == DELETE)
    {
        // Process DELETE request
        // _response.handleDelete();
        handelDelete();
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        setResponse(response);
    }
    else
    {
        // Method not supported        std::string response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        std::string response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        setResponse(response);
    }
}

bool isMatch(const std::string& uri, const std::string& location) {
        if (uri.compare(0, location.size(), location) != 0)
            return false;

        if (uri.size() == location.size())
            return true;

        if (location[location.size() - 1] == '/')
            return true;

        return uri[location.size()] == '/';
    
}

void Client::handelGet() {
    LocationConfig* best = NULL;
    std::string uri = Request::getUri();
    const std::vector<LocationConfig> locations = _config.getLocations();
    for (size_t i = 0; i < locations.size(); i++)
    {
        if (isMatch(uri, locations[i].getPath()))
        {
            if (!best ||
                locations[i].getPath().size() >
                best->getPath().size())
            {
                best = &locations[i];
            }
        }
    }
}

Client::Client(int fd, const Config &config) : _fd(fd), _config(config) {
    _complete = false;
}

int Client::getFd() const {
    return _fd;
}

const Config &Client::getConfig() const {
    return _config;
}

size_t getContentLength(const std::string& headers)
{
    size_t pos = headers.find("Content-Length:");
    if (pos == std::string::npos)
        return 0;
    pos += 15;
    while (pos < headers.size() && headers[pos] == ' ')
        pos++;
    return std::strtoul(headers.c_str() + pos, NULL, 10);
}

// bool Client::isRequestComplete() const
// {
    
// }

// void Client::displayrequest()
// {
//     _request.displayrequest();
// }

// void Client::parseRequest()
// {
//     _request.appendData(_readBuffer);
//     _request.parse();
// }

