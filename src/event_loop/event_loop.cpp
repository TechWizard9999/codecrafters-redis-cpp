#include "event_loop.h"
#include "event_handler.h"

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <vector>

EventLoop::EventLoop() {}

EventLoop::~EventLoop() {
  if (kq_fd_ >= 0) {
    close(kq_fd_);
  }
}

bool EventLoop::init() {
  kq_fd_ = kqueue();
  if (kq_fd_ < 0) {
    std::cerr << "Failed to create kqueue\n";
    return false;
  }
  return true;
}

void EventLoop::addReadEvent(int fd, EventHandler* handler) {
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, handler);
  kevent(kq_fd_, &ev, 1, nullptr, 0, nullptr);
}

void EventLoop::addWriteEvent(int fd, EventHandler* handler) {
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, handler);
  kevent(kq_fd_, &ev, 1, nullptr, 0, nullptr);
}

void EventLoop::removeReadEvent(int fd) {
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  kevent(kq_fd_, &ev, 1, nullptr, 0, nullptr);
}

void EventLoop::removeWriteEvent(int fd) {
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
  kevent(kq_fd_, &ev, 1, nullptr, 0, nullptr);
}

void EventLoop::removeAllEvents(int fd) {
  removeReadEvent(fd);
  removeWriteEvent(fd);
}

void EventLoop::run() {
  is_running_ = true;
  std::vector<struct kevent> events(32);

  while (is_running_) {
    int n = kevent(kq_fd_, nullptr, 0, events.data(), events.size(), nullptr);
    if (n < 0) {
      std::cerr << "kevent error\n";
      break;
    }

    for (int i = 0; i < n; ++i) {
      EventHandler* handler = static_cast<EventHandler*>(events[i].udata);
      if (!handler) continue;

      // Note: EV_ERROR could also be checked here, but basic handling calls the handler methods.
      if (events[i].filter == EVFILT_READ) {
        handler->handleRead();
      } else if (events[i].filter == EVFILT_WRITE) {
        handler->handleWrite();
      }
    }
  }
}
