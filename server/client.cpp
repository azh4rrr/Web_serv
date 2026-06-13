#include "client.hpp"


// bool Client::isheaderComplete() const
// {
//     return _readBuffer.find("\r\n\r\n") != std::string::npos;
// }

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

