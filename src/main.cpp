#include <iostream>
#include <memory>
#include "server/server.h"
#include "server/request_handler.h"
#include "event_loop/event_loop.h"

class PingHandler : public RequestHandler {
public:
  std::string handle(const std::string& request) override {
    (void)request;
    return "+PONG\r\n";
  }
};

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  auto handler = std::make_shared<PingHandler>();
  EventLoop loop;
  if (!loop.init()) {
    return 1;
  }

  Server server(&loop, handler);
  if (!server.start()) {
    return 1;
  }

  std::cout << "Server started. Waiting for clients to connect...\n";
  std::cout << "Logs from your program will appear here!\n";

  loop.run();

  return 0;
}
