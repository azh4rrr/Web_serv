#pragma once
#include <string>
#include "request.hpp"
#include "../config/Config.hpp"

class Response {
private:
    std::string _rawResponse;

    std::string _version;
    std::string _body;
    size_t _contentLength;
    std::map<std::string, std::string> _header;
    std::string contentType;
    std::string _statusCode;
    std::string _statusMessage;

public:
    Response() : _rawResponse("") {};


    void responseprocces();
    
    void setRawResponse(const std::string& response) {
        _rawResponse = response;
    }
    const std::string& getRawResponse() const {
        return _rawResponse;
    }
    void setStatusCode(const std::string& code) {
        _statusCode = code;
    }
    void setStatusMessage(const std::string& message) {
        _statusMessage = message;
    }
    void setHeader(const std::string& key, const std::string& value) {
        _header[key] = value;
    }
    void setBody(const std::string& body) {
        _body = body;
    }
    void setversion(const std::string& version) {
        _version = version;
    }
    void buildResponse() {
        _rawResponse = _version + " " + _statusCode + " " + _statusMessage + "\r\n";
        for (std::map<std::string, std::string>::const_iterator it = _header.begin(); it != _header.end(); ++it) {
            _rawResponse += it->first + ": " + it->second + "\r\n";
        }
        _rawResponse += "\r\n" + _body;
    }
};