#include "client.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

void Client::processResponse() {

  if (Request::getMethod() == GET) {
    handelGET();
    return;
  } else if (Request::getMethod() == POST) {
    handelPOST();
  } else if (Request::getMethod() == DELETE) {
    handelDELETE();
  }
}

bool isMatch(const std::string &uri, const std::string &location) {

  if (uri.compare(0, location.size(), location) != 0)
    return false;
  if (uri.size() == location.size())
    return true;
  if (location[location.size() - 1] == '/')
    return true;
  return uri[location.size()] == '/';
}

// bool Client::allowedMethod(const std::string &method) const
// {

// }

void Client::setTargetPath() {

  const std::vector<LocationConfig> &locations = _config.getLocations();
  std::string uri = getUri();
  std::string targetPath;

  for (size_t i = 0; i < locations.size(); i++) {

    std::cout << "Checking location" << std::endl;
    if (isMatch(uri, locations[i].getPath())) {
      if (_targetLocation.getPath().empty() ||
          locations[i].getPath().size() > _targetLocation.getPath().size()) {
        _targetLocation = locations[i];
      }
    }
  }
  if (_targetLocation.getPath().empty()) {
  }
  if (_targetLocation.isMethodAllowed(getMethod())) {
    std::cout << "Method " << getMethod() << " is allowed for location "
              << _targetLocation.getPath() << std::endl;
  } else {
    std::cout << "Method " << getMethod() << " is NOT allowed for location "
              << _targetLocation.getPath() << std::endl;
  }

  std::string root;
  if (!_targetLocation.getRoot().empty())
    root = _targetLocation.getRoot();
  else
    root = _config.getRoot();
  // std::string path;
  if (_targetLocation.getPath() == "/")
    _targetLocation.setPath(root + uri);

  else
    _targetLocation.setPath(root + uri.substr(_targetLocation.getPath().size()));
  // _targetPath = root + uri.substr(_targetLocation.getPath().size());
}

std::string readFile(const std::string &filepath) {
  // Implement file reading logic here, e.g., open the file, read its contents
  // into a string, and return it
  std::string content;
  std::ifstream file(filepath.c_str());
  if (!file.is_open()) {
    std::cout << "Failed to open file: " << filepath << std::endl;
    return "";
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  content = ss.str();
  if (content.empty()) {
    std::cout << "File is empty: " << filepath << std::endl;
  }
  return content;
  // ...
}

void Client::sendFile(const std::string &filepath) {
  // Implement file sending logic here, e.g., read the file and send its
  // contents to the client

  std::cout << "Sending file: " << filepath << std::endl;
  if (access(filepath.c_str(), R_OK) != 0) {
    std::cout << "File is not readable: " << filepath << std::endl;
    // sendError(403); return;
  }
  // read file, send 200
  std::string buffer = readFile(filepath);
  if (buffer.empty()) {
    std::cout << "No content to send for: " << filepath << std::endl;
    std::string notFound =
        "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    _response.setRawResponse(notFound);
    return;
  }
  _response.setStatusCode("200");
  _response.setversion("HTTP/1.1");
  _response.setStatusMessage("OK");
  _response.setHeader("Content-Length", std::to_string(buffer.size()));
  _response.setHeader("Content-Type",
                      "text/html"); // You may want to determine the content
  _response.setBody(buffer);
  _response.buildResponse();
  std::cout << _response.getRawResponse() << std::endl;
  // readyToSend(getFd());
  // std::string response = "HTTP/1.1 200 OK\r\nContent

  // int fileFd = open(filepath.c_str(), O_RDONLY);
  // if (fileFd < 0) {
  // std::cout << "Failed to open file: " << filepath << std::endl;
  // sendError(500); return;
}

void Client::handelGET() {


  setTargetPath();
  const std::string filepath = _targetLocation.getPath();
  // std::cout << "Target file path: " << filepath << std::endl;
  // std::string index
  // match server name

  // std::cout << "Target file path: " << filepath << std::endl;
  struct stat st;
  if (stat(filepath.c_str(), &st) != 0) {
    std::cout << "File not found: " << filepath << std::endl;
    // sendError(404); return;
  }

  if (S_ISREG(st.st_mode)) {
    std::cout << "File exists and is a regular file: " << filepath <<
    std::endl;
  // File exists and is a regular file, check if it's readable

    if (access(filepath.c_str(), R_OK) != 0) {
        std::cout << "File is not readable: " << filepath << std::endl;
        // sendError(403); return;
    }
      // std::cout << "File is readable: " << filepath << std::endl;
      // sendError(403); return;

      sendFile(filepath);
      // sendError(403); return;
      //   return;
  }

  if (S_ISDIR(st.st_mode)) {

    std::string index = _targetLocation.getIndex();
    // std::cout << "Index file specified: " << index << std::endl;
    // if (!index.empty()) {
    //     std::string indexPath = filepath + "/" + index;
    //     if (stat(indexPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
    //         std::cout << "Index file found: " << indexPath << std::endl;
    //         sendFile(indexPath);
    //         return;
    //     }
    // }

    std::cout << "Path is a directory: " << filepath << std::endl;
    sendFile(filepath + index);
    // Handle directory case, e.g., check for index file or autoindex
    // if (_targetLocation.getAutoindex()) {
    //     std::cout << "Autoindex is enabled for directory: " << filepath <<
    //     std::endl;
    //     // Generate and send autoindex page
    // } else {
    //     // sendfile(filepath);
    //     std::cout << "Autoindex is disabled for directory: " << filepath
    // <<
    //     std::endl;
    //     // sendError(403); return;
    // }
  }

  // read file, send 200
  // sendFile(filepath);
}

Client::Client(int fd, const Config &config) : _fd(fd), _config(config) {
  _complete = false;
}

int Client::getFd() const { return _fd; }

const Config &Client::getConfig() const { return _config; }

size_t getContentLength(const std::string &headers) {
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
