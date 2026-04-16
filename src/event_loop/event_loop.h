#pragma once

class EventHandler;

class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  bool init();
  void addReadEvent(int fd, EventHandler* handler);
  void addWriteEvent(int fd, EventHandler* handler);
  void removeReadEvent(int fd, EventHandler* handler);
  void removeWriteEvent(int fd, EventHandler* handler);
  void removeAllEvents(int fd, EventHandler* handler);
  void run();

private:
  int fd_ = -1;
  bool is_running_ = false;
};
