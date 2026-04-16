#pragma once

#include "../event_loop/event_handler.h"
#include <string>
#include <memory>

class RequestHandler;
class EventLoop;

class Server : public EventHandler {
public:
  Server(EventLoop* loop, std::shared_ptr<RequestHandler> handler);
  ~Server() override;

  bool start();

  // EventHandler overrides
  void handleRead() override;
  void handleWrite() override;

private:
  void closeSocket(int& fd) const;

  int server_fd_ = -1;
  EventLoop* loop_;
  std::shared_ptr<RequestHandler> handler_;
};
