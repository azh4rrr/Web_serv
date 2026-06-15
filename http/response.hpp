#pragma once
#include <string>
#include "request.hpp"
#include "../config/Config.hpp"

class Response {
private:

    std::string _rawResponse;
public:
    Response() {};


    void responseprocces();
    void setRawResponse(const std::string& response) {
        _rawResponse = response;
    }
    const std::string& getRawResponse() const {
        return _rawResponse;
    }
};