#include "request.hpp"

// void Request::appendData(const std::string& data)
// {
//     _rawRequest.append(data);
// }

// void Request::parse()
// {
//     size_t headerEnd = _rawRequest.find("\r\n\r\n");

//     if (headerEnd == std::string::npos)
//         throw std::runtime_error("Bad Request");

//     std::string headerPart = _rawRequest.substr(0, headerEnd);

//     std::string bodyPart = _rawRequest.substr(headerEnd + 4);

//     size_t firstLineEnd = headerPart.find("\r\n");

//     if (firstLineEnd == std::string::npos)
//         throw std::runtime_error("Bad Request");
//     parseRequestLine(headerPart.substr(0, firstLineEnd));
//     parseHeaders(headerPart.substr(firstLineEnd + 2));
//     parseBody(bodyPart);
// }

HttpMethod Request::stringToMethod(const std::string& method){

    if (method == "GET")
        return GET;
    else if (method == "POST")
        return POST;
    else if (method == "DELETE")
        return DELETE;
    else
        return UNKNOWN;
}

void Request::parseRequestLine(const std::string& line)
{
    size_t pos1 = line.find(' ');

    if (pos1 == std::string::npos)
        throw std::runtime_error("400");

    size_t pos2 = line.find(' ', pos1 + 1);

    if (pos2 == std::string::npos)
        throw std::runtime_error("400");

    std::string method =line.substr(0, pos1);

    _uri = line.substr(pos1 + 1,pos2 - pos1 - 1);

    _version = line.substr(pos2 + 1);

    _method = stringToMethod(method);

    if (_method == UNKNOWN)
        throw std::runtime_error("501");
    if (_version != "HTTP/1.0" && _version != "HTTP/1.1")
        throw std::runtime_error("505");
    if (_uri.empty())
        throw std::runtime_error("400");
}

bool Request::validkey(const std::string& key) const
{
    if (key.empty())
        return false;
    if (key.find(' ') != std::string::npos)
        return false;
    return true;
}

void Request::parseHeaders(const std::string& headersPart)
{
    size_t start = 0;

    while (start < headersPart.size())
    {
        size_t end = headersPart.find("\r\n", start);
        if (end == std::string::npos)
            end = headersPart.size();
        std::string line = headersPart.substr(start,end - start);

        size_t colon = line.find(':');
        if (colon == std::string::npos)
            throw std::runtime_error("400");

        std::string key = line.substr(0, colon);
        // i want changer the key to lowercase
        std::string value = line.substr(colon + 1);
        // Trim leading spaces from value
        while (!value.empty() && value[0] == ' ')
            value.erase(0, 1);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        if (validkey(key))
            _headers[key] = value;
        else
            throw std::runtime_error("400");
        start = end + 2;
    }
}

void Request::parseBody(const std::string& bodyPart)
{
    _body = bodyPart;
}

#include <iostream>
void Request::displayrequest()
{
    std::cout << "Method: " << _method << std::endl;
    std::cout << "URI: " << _uri << std::endl;
    std::cout << "Version: " << _version << std::endl;
    std::cout << "Headers:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    if (!_body.empty())
        std::cout << "Body: " << _body << std::endl;
}

bool Request::isheaderComplete()
{
    if(!_complete)
    {
        size_t pos = _rawRequest.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            _complete = true;        
            parseRequest();
        }
    }
        // _complete = _rawRequest.find("\r\n\r\n") != std::string::npos;
    return _complete;
}


void Request::parseRequest()
{
    std::cout << "Parsing request..." << std::endl;
    size_t headerEnd = _rawRequest.find("\r\n\r\n");

    if (headerEnd == std::string::npos)
        throw std::runtime_error("Bad Request");

    std::string headerPart = _rawRequest.substr(0, headerEnd);

    std::string bodyPart = _rawRequest.substr(headerEnd + 4);

    size_t firstLineEnd = headerPart.find("\r\n");

    if (firstLineEnd == std::string::npos)
        throw std::runtime_error("Bad Request");
    parseRequestLine(headerPart.substr(0, firstLineEnd));
    parseHeaders(headerPart.substr(firstLineEnd + 2));
    parseBody(bodyPart);
}

bool Request::isRequestComplete() const
{
    // std::cout << "Checking if request is complete for method: " << _method << std::endl;
    if(_method == POST)
    {
        
        std::map<std::string, std::string>::const_iterator it = _headers.find("Content-Length");
        if (it == _headers.end())
            // bad request if POST but no Content-Length header
            throw std::runtime_error("400");
        size_t contentLength = std::strtoul(it->second.c_str(), NULL, 10);
        std::cout << "Content-Length: " << contentLength << std::endl;
        return _body.size() >= contentLength;
    }
    return true; 
}