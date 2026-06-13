#pragma once


#include <string>
#include <map>

enum HttpMethod
{
    GET,
    POST,
    DELETE,
    UNKNOWN
};

class Request
{
private:
    std::string _rawRequest;
    HttpMethod _method;
    std::string _uri;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _body;
    
    void parseRequestLine(const std::string& line);
    void parseHeaders(const std::string& headersPart);
    void parseBody(const std::string& bodyPart);
    
public:
    bool _complete;
    Request() : _rawRequest("") {};
    void appendrequest(const std::string& data) {
        _rawRequest.append(data);
    }
    HttpMethod getMethod() const;
    const std::string& getUri() const;
    const std::string& getVersion() const;
    const std::string& getBody() const;
    const std::map<std::string, std::string>& getHeaders() const;
    std::string getHeader(const std::string& key) const;
    void displayrequest();
    bool validkey(const std::string& key) const;

    void parseRequest();
    HttpMethod stringToMethod(const std::string& method);
    bool isheaderComplete();
    bool isRequestComplete() const;

    // bool hasHeader(const std::string& key) const;
};
