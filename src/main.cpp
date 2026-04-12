#include <iostream>
#include "server.h"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  Server server;
  if (!server.start()) {
    return 1;
  }

  std::cout << "Waiting for a client to connect...\n";
  std::cout << "Logs from your program will appear here!\n";

  if (!server.acceptClient()) {
    return 1;
  }

  std::cout << "Client connected\n";
  return server.handleClientSession() ? 0 : 1;
}
