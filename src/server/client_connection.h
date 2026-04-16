#pragma once
#include "../event_loop/event_handler.h"
#include <string>
#include <memory>

class EventLoop;
class RequestHandler;

class ClientConnection : public EventHandler {
public:
  ClientConnection(int fd, EventLoop* loop, std::shared_ptr<RequestHandler> handler);
  ~ClientConnection() override;

  void handleRead() override;
  void handleWrite() override;

private:
  int fd_;
  EventLoop* loop_;
  std::shared_ptr<RequestHandler> handler_;
  std::string write_buffer_;
};
