#include "server.h"
#include "request_handler.h"
#include "../event_loop/event_loop.h"
#include "client_connection.h"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

namespace {

constexpr int kPort = 6379;
constexpr int kConnectionBacklog = 5;

}

Server::Server(EventLoop* loop, std::shared_ptr<RequestHandler> handler) 
  : loop_(loop), handler_(std::move(handler)) {}

Server::~Server() {
  if (server_fd_ >= 0) {
    loop_->removeAllEvents(server_fd_);
  }
  closeSocket(server_fd_);
}

bool Server::start() {
  server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd_ < 0) {
    std::cerr << "Failed to create server socket\n";
    return false;
  }

  // Set non-blocking
  int flags = fcntl(server_fd_, F_GETFL, 0);
  fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);

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

  // Register server socket with the event loop for reading (incoming connections)
  loop_->addReadEvent(server_fd_, this);

  return true;
}

void Server::handleRead() {
  sockaddr_in client_addr{};
  socklen_t client_addr_len = sizeof(client_addr);
  int client_fd = accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
  
  if (client_fd >= 0) {
    // Set client socket to non-blocking
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    
    // Create new connection state. It registers itself to the event loop.
    new ClientConnection(client_fd, loop_, handler_);
    std::cout << "Client connected (FD " << client_fd << ")\n";
  } else {
    // Typically EWOULDBLOCK, handle it gracefully by just returning
    // std::cerr << "accept failed\n";
  }
}

void Server::handleWrite() {
  // Server socket doesn't write.
}

void Server::closeSocket(int& fd) const {
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
}
