#pragma once

#include <string>

class Server {
public:
  Server() = default;
  ~Server();

  bool start();
  bool acceptClient();
  bool handleClientSession();

private:
  std::string readRequest() const;
  std::string handleCommand(const std::string& request) const;
  bool writeResponse(const std::string& response) const;
  void closeSocket(int& fd) const;

  int server_fd_ = -1;
  int client_fd_ = -1;
};
