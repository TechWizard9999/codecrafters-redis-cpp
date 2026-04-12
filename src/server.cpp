#include "server.h"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace {

constexpr int kPort = 6379;
constexpr int kConnectionBacklog = 5;

}

Server::~Server() {
  closeSocket(client_fd_);
  closeSocket(server_fd_);
}

bool Server::start() {
  server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd_ < 0) {
    std::cerr << "Failed to create server socket\n";
    return false;
  }

  int reuse = 1;
  if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    closeSocket(server_fd_);
    return false;
  }

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(kPort);

  if (bind(server_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 6379\n";
    closeSocket(server_fd_);
    return false;
  }

  if (listen(server_fd_, kConnectionBacklog) != 0) {
    std::cerr << "listen failed\n";
    closeSocket(server_fd_);
    return false;
  }

  return true;
}

bool Server::acceptClient() {
  sockaddr_in client_addr{};
  socklen_t client_addr_len = sizeof(client_addr);
  client_fd_ = accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
  if (client_fd_ < 0) {
    std::cerr << "accept failed\n";
    return false;
  }

  return true;
}

bool Server::handleClientSession() {
  while (true) {
    std::string request = readRequest();
    if (request.empty()) {
      return true;
    }

    std::string response = handleCommand(request);
    if (!writeResponse(response)) {
      return false;
    }
  }
}

std::string Server::readRequest() const {
  char buffer[1024];
  ssize_t bytes_read = read(client_fd_, buffer, sizeof(buffer));
  if (bytes_read < 0) {
    std::cerr << "read failed\n";
    return {};
  }

  if (bytes_read == 0) {
    return {};
  }

  return {buffer, static_cast<std::size_t>(bytes_read)};
}

std::string Server::handleCommand(const std::string& request) const {
  (void)request;
  return "+PONG\r\n";
}

bool Server::writeResponse(const std::string& response) const {
  ssize_t bytes_written = write(client_fd_, response.c_str(), response.size());
  if (bytes_written < 0) {
    std::cerr << "write failed\n";
    return false;
  }

  return static_cast<std::size_t>(bytes_written) == response.size();
}

void Server::closeSocket(int& fd) const {
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
}
