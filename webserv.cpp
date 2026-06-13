#include "Webserv.hpp"
#include "config/ParssingConf.hpp"
#include "server/server.hpp"
#include "server/client.hpp"
// #include <algorithm>
#include <iostream>
#include <string>
#include <unistd.h>

bool Webserv::is_server(int fd) const {
    for (size_t i = 0; i < _servers.size(); ++i) {
        if (_servers[i].getFd() == fd) {
            return true;
        }
    }
    return false;
}

void Webserv::setupServers(const std::string &configFile)
{
    ParssingConf parser;
    parser.parseConfig(configFile);
    const std::vector<Config> &configs = parser.getConfigs();

    for (size_t i = 0; i < configs.size(); ++i)
    {
        Server server(configs[i]);
        _servers.push_back(server);
        pollfd pfd;
        pfd.fd = server.getFd();
        pfd.events = POLLIN;
        _pollfds.push_back(pfd);
    }
    std::cout << "Servers set up successfully" << std::endl;
}

void Webserv::newConnection(int serverFd)
{
    int clientFd = accept(serverFd, NULL, NULL);
    if (clientFd < 0)
    {
        throw std::runtime_error("Failed to accept new connection");
    }
    std::cout << "New connection accepted, client fd: " << clientFd << std::endl;
    Client client(clientFd, getServerByFd(serverFd)->getConfig());
    _clients.push_back(client);
    pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    _pollfds.push_back(pfd);
}

void Webserv::readFromClient(int clientFd)
{
    Client* client = getClientByFd(clientFd);
    // Request& request = client->getRequest();
    if (!client)
        throw std::runtime_error("Client not found");

    char buffer[10];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
    if (bytesRead < 0)
    {
        throw std::runtime_error("Failed to read from client");
    }
    else if (bytesRead == 0)
    {
        removeClient(clientFd);
        return;
    }
    std::cout << "Received data from client fd " << clientFd << ": " << std::string(buffer, bytesRead) << std::endl;
    client->appendrequest(std::string(buffer, bytesRead));
    // client->_request.appendrequest(std::string(buffer, bytesRead));
    if (client->isheaderComplete())
    {
        if(client->isRequestComplete())
        {
            std::cout << "Full request received from client fd " << clientFd << std::endl;
            // Process the request and prepare the response
            //display the request for testing
            client->displayrequest();
            std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
            client->setResponse(response);
            readyToSend(clientFd);
        }
        else
        {
            std::cout << "Headers complete but body not fully received for client fd " << clientFd << std::endl;
            return; // Wait for more data to complete the request   
        }

    }

}

void Webserv::Start()
{
    while (true)
    {
        int ret = poll(_pollfds.data(), _pollfds.size(), -1);
        if (ret < 0)
        {
            throw std::runtime_error("Poll failed");
        }
        for (size_t i = 0; i < _pollfds.size(); ++i)
        {
            if (_pollfds[i].revents & POLLIN)
            {
                // std::cout << "POLLIN event on fd " << _pollfds[i].fd << std::endl;
                if (is_server(_pollfds[i].fd))
                {
                    newConnection(_pollfds[i].fd);
                }
                else
                {
                    readFromClient(_pollfds[i].fd);
                }
            }
            else if (_pollfds[i].revents & POLLOUT)
            {
                Client* client = getClientByFd(_pollfds[i].fd);
                if (client)
                {
                    const std::string& response = client->getResponse().getRawResponse();
                    ssize_t bytesSent = send(client->getFd(), response.c_str(), response.size(), 0);
                    if (bytesSent < 0)
                    {
                        throw std::runtime_error("Failed to send response to client");
                    }
                    removeClient(client->getFd());
                }
            }
            else if (_pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                    std::cout << "Error event on fd " << _pollfds[i].fd << std::endl;
                    // Handle error event
            }

        }
    }
}

Server* Webserv::getServerByFd(int fd)
{
    for (size_t i = 0; i < _servers.size(); ++i)
    {
        if (_servers[i].getFd() == fd)
        {
            return &_servers[i];
        }
    }
    return nullptr;
}

Client* Webserv::getClientByFd(int fd)
{
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i].getFd() == fd)
        {
            return &_clients[i];
        }
    }
    return nullptr;
}

pollfd* Webserv::getPollfdByFd(int fd)
{
    for (size_t i = 0; i < _pollfds.size(); ++i)
    {
        if (_pollfds[i].fd == fd)
        {
            return &_pollfds[i];
        }
    }
    return nullptr;
}

void Webserv::removeClient(int clientFd)
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getFd() == clientFd)
        {
            _clients.erase(it);
            break;
        }
    }
    for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
    {
        if (it->fd == clientFd)
        {
            _pollfds.erase(it);
            break;
        }
    }
    close(clientFd);
}

void Webserv::readyToSend(int clientFd)
{
   pollfd* pfd = getPollfdByFd(clientFd);
    if (pfd)
    {
        pfd->events = POLLOUT;
    }
}