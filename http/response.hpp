#pragma once
#include <string>

class Response {
private:
    std::string _rawResponse;
public:
    void setRawResponse(const std::string& response) {
        _rawResponse = response;
    }
    const std::string& getRawResponse() const {
        return _rawResponse;
    }
};