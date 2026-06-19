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


#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <sstream>

// ─── Helper: format file size ────────────────────────────────────────────────

static std::string formatSize(off_t size)
{
    std::ostringstream ss;
    if (size < 1024)
        ss << size << " B";
    else if (size < 1024 * 1024)
        ss << size / 1024 << " KB";
    else
        ss << size / (1024 * 1024) << " MB";
    return ss.str();
}

// ─── Helper: format timestamp ────────────────────────────────────────────────

static std::string formatTime(time_t t)
{
    char buf[32];
    struct tm* tm = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tm);
    return std::string(buf);
}

// ─── Core: build HTML directory listing ──────────────────────────────────────

std::string Client::buildAutoIndex(const std::string& dirPath, const std::string& uri)
{
    // 1. Open the directory
    DIR* dir = opendir(dirPath.c_str());
    if (!dir)
        return "";  // caller will send 403/500

    // 2. Collect all entries
    struct dirent* entry;
    std::vector<std::string> dirs;   // directories first
    std::vector<std::string> files;  // then files

    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        // skip current dir entry
        if (name == ".")
            continue;

        std::string fullPath = dirPath;
        if (fullPath[fullPath.size() - 1] != '/')
            fullPath += '/';
        fullPath += name;

        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0)
            continue;  // skip entries we can't stat

        if (S_ISDIR(st.st_mode))
            dirs.push_back(name);
        else
            files.push_back(name);
    }
    closedir(dir);

    // 3. Sort both lists alphabetically
    std::sort(dirs.begin(), dirs.end());
    std::sort(files.begin(), files.end());

    // 4. Build HTML
    std::ostringstream html;

    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "  <meta charset=\"UTF-8\">\n"
         << "  <title>Index of " << uri << "</title>\n"
         << "  <style>\n"
         << "    body { font-family: monospace; padding: 20px; }\n"
         << "    h1   { border-bottom: 1px solid #ccc; padding-bottom: 8px; }\n"
         << "    table{ border-collapse: collapse; width: 100%; }\n"
         << "    th   { text-align: left; padding: 6px 16px; border-bottom: 2px solid #ccc; }\n"
         << "    td   { padding: 4px 16px; }\n"
         << "    tr:hover { background: #f5f5f5; }\n"
         << "    a    { text-decoration: none; color: #0366d6; }\n"
         << "    a:hover { text-decoration: underline; }\n"
         << "    .dir { color: #6f42c1; }\n"
         << "  </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <h1>Index of " << uri << "</h1>\n"
         << "  <table>\n"
         << "    <tr>\n"
         << "      <th>Name</th>\n"
         << "      <th>Last Modified</th>\n"
         << "      <th>Size</th>\n"
         << "    </tr>\n";

    // 5. Parent directory link (except if we're at root)
    if (uri != "/")
    {
        html << "    <tr>\n"
             << "      <td><a href=\"../\">../</a></td>\n"
             << "      <td>-</td>\n"
             << "      <td>-</td>\n"
             << "    </tr>\n";
    }

    // 6. Directories first
    for (size_t i = 0; i < dirs.size(); i++)
    {
        std::string fullPath = dirPath;
        if (fullPath[fullPath.size() - 1] != '/')
            fullPath += '/';
        fullPath += dirs[i];

        struct stat st;
        stat(fullPath.c_str(), &st);

        html << "    <tr>\n"
             << "      <td class=\"dir\">"
             << "<a href=\"" << dirs[i] << "/\">" << dirs[i] << "/</a>"
             << "</td>\n"
             << "      <td>" << formatTime(st.st_mtime) << "</td>\n"
             << "      <td>-</td>\n"
             << "    </tr>\n";
    }

    // 7. Files
    for (size_t i = 0; i < files.size(); i++)
    {
        std::string fullPath = dirPath;
        if (fullPath[fullPath.size() - 1] != '/')
            fullPath += '/';
        fullPath += files[i];

        struct stat st;
        stat(fullPath.c_str(), &st);

        html << "    <tr>\n"
             << "      <td>"
             << "<a href=\"" << files[i] << "\">" << files[i] << "</a>"
             << "</td>\n"
             << "      <td>" << formatTime(st.st_mtime) << "</td>\n"
             << "      <td>" << formatSize(st.st_size)  << "</td>\n"
             << "    </tr>\n";
    }

    html << "  </table>\n"
         << "</body>\n"
         << "</html>\n";

    return html.str();
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


void Client::findTargetLocation() {
  const std::vector<LocationConfig> &locations = _config.getLocations();
  std::string uri = getUri();

  for (size_t i = 0; i < locations.size(); i++) {
    if (isMatch(uri, locations[i].getPath())) {
      if (_targetLocation.getPath().empty() ||
          locations[i].getPath().size() > _targetLocation.getPath().size()) {
        _targetLocation = locations[i];
      }
    }
  }
}

void Client::setTargetPath() {

  std::string uri = getUri();
  std::string root = _targetLocation.getRoot();
  std::string index = _targetLocation.getIndex();

  if (uri == _targetLocation.getPath()) {
    if (!index.empty()) {
      if (root[root.size() - 1] != '/')
          root += '/';
      _targetLocation.setPath(root + index);
    } else {
      _targetLocation.setPath(root);
    }
  } else {
    std::string relativePath = uri.substr(_targetLocation.getPath().size());
    if (!relativePath.empty() && relativePath[0] != '/')
      relativePath = "/" + relativePath;
    _targetLocation.setPath(root + relativePath);
  }
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

  findTargetLocation();
  setTargetPath();
  const std::string filepath = _targetLocation.getPath();
  struct stat st;
  if (stat(filepath.c_str(), &st) != 0) {
    std::cout << "File not found: " << filepath << std::endl;
  }

  if (S_ISREG(st.st_mode)) {
    std::cout << "Path is a regular file: " << filepath << std::endl;
    if (access(filepath.c_str(), R_OK) != 0) {
        std::cout << "File is not readable: " << filepath << std::endl;
    }
    sendFile(filepath);
  }
  if (S_ISDIR(st.st_mode)) {
    if (_targetLocation.getAutoindex()) {

        if (_targetLocation.getAutoindex())
    {
        // std::string dirPath = filepath;
        // if (dirPath.back() != '/') dirPath += '/';

        std::string body = buildAutoIndex(_targetLocation.getPath(), getUri());

        // if (body.empty())
        // {
        //     sendError(500);   // opendir failed
        //     return;
        // }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/html\r\n"
                 << "Content-Length: " << body.size() << "\r\n"
                 << "Connection: keep-alive\r\n"
                 << "\r\n"
                 << body;

        // setResponse(response.str());
        setResponse(response.str());
        return;
    }
      std::cout << "Autoindex is enabled for directory: " << filepath
                << std::endl;
      // Implement autoindex logic here, e.g., generate a directory listing
      // and send it to the client
    } else {
      std::cout << "Autoindex is disabled for directory: " << filepath
                << std::endl;
      // Handle the case where autoindex is off, e.g., send a 403 Forbidden
      // response or redirect to an index file if specified
    }
    // std::cout << "Path is a directory: " << filepath << std::endl;
    // std::string index = _targetLocation.getIndex();
    // std::cout << "Index file specified: " << index << std::endl;
  }
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
