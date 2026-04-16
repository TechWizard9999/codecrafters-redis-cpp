#pragma once

class EventHandler {
public:
  virtual ~EventHandler() = default;
  virtual void handleRead() = 0;
  virtual void handleWrite() = 0;
};
