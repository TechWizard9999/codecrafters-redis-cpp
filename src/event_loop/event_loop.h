#pragma once

class EventHandler;

class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  bool init();
  void addReadEvent(int fd, EventHandler* handler);
  void addWriteEvent(int fd, EventHandler* handler);
  void removeReadEvent(int fd);
  void removeWriteEvent(int fd);
  void removeAllEvents(int fd);
  void run();

private:
  int kq_fd_ = -1;
  bool is_running_ = false;
};
