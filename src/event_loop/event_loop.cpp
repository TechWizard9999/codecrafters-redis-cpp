#include "event_loop.h"
#include "event_handler.h"

#include <unistd.h>
#include <iostream>
#include <vector>
#include <cerrno>

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#define USE_KQUEUE 1
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#elif defined(__linux__)
#define USE_EPOLL 1
#include <sys/epoll.h>
#else
#error "Unsupported operating system"
#endif

EventLoop::EventLoop() {}

EventLoop::~EventLoop() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

bool EventLoop::init() {
#ifdef USE_KQUEUE
  fd_ = kqueue();
  if (fd_ < 0) {
    std::cerr << "Failed to create kqueue\n";
    return false;
  }
#elif defined(USE_EPOLL)
  fd_ = epoll_create1(0);
  if (fd_ < 0) {
    std::cerr << "Failed to create epoll\n";
    return false;
  }
#endif
  return true;
}

void EventLoop::addReadEvent(int fd, EventHandler* handler) {
#ifdef USE_KQUEUE
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, handler);
  kevent(fd_, &ev, 1, nullptr, 0, nullptr);
#elif defined(USE_EPOLL)
  struct epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.ptr = handler;
  if (epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev) < 0 && errno == EEXIST) {
    ev.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
  }
#endif
}

void EventLoop::addWriteEvent(int fd, EventHandler* handler) {
#ifdef USE_KQUEUE
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, handler);
  kevent(fd_, &ev, 1, nullptr, 0, nullptr);
#elif defined(USE_EPOLL)
  struct epoll_event ev{};
  ev.events = EPOLLIN | EPOLLOUT;
  ev.data.ptr = handler;
  epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
#endif
}

void EventLoop::removeReadEvent(int fd, EventHandler* handler) {
#ifdef USE_KQUEUE
  (void)handler;
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  kevent(fd_, &ev, 1, nullptr, 0, nullptr);
#elif defined(USE_EPOLL)
  struct epoll_event ev{};
  ev.events = EPOLLOUT;
  ev.data.ptr = handler;
  epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
#endif
}

void EventLoop::removeWriteEvent(int fd, EventHandler* handler) {
#ifdef USE_KQUEUE
  (void)handler;
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
  kevent(fd_, &ev, 1, nullptr, 0, nullptr);
#elif defined(USE_EPOLL)
  struct epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.ptr = handler;
  epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
#endif
}

void EventLoop::removeAllEvents(int fd, EventHandler* handler) {
#ifdef USE_KQUEUE
  removeReadEvent(fd, handler);
  removeWriteEvent(fd, handler);
#elif defined(USE_EPOLL)
  (void)handler;
  epoll_ctl(fd_, EPOLL_CTL_DEL, fd, nullptr);
#endif
}

void EventLoop::run() {
  is_running_ = true;

#ifdef USE_KQUEUE
  std::vector<struct kevent> events(32);
#elif defined(USE_EPOLL)
  std::vector<struct epoll_event> events(32);
#endif

  while (is_running_) {
#ifdef USE_KQUEUE
    int n = kevent(fd_, nullptr, 0, events.data(), events.size(), nullptr);
#elif defined(USE_EPOLL)
    int n = epoll_wait(fd_, events.data(), events.size(), -1);
#endif

    if (n < 0) {
      if (errno == EINTR) continue;
      std::cerr << "Event poll error\n";
      break;
    }

    for (int i = 0; i < n; ++i) {
#ifdef USE_KQUEUE
      EventHandler* handler = static_cast<EventHandler*>(events[i].udata);
      if (!handler) continue;

      if (events[i].filter == EVFILT_READ) {
        handler->handleRead();
      } else if (events[i].filter == EVFILT_WRITE) {
        handler->handleWrite();
      }
#elif defined(USE_EPOLL)
      EventHandler* handler = static_cast<EventHandler*>(events[i].data.ptr);
      if (!handler) continue;

      if (events[i].events & (EPOLLIN | EPOLLERR | EPOLLHUP)) {
        handler->handleRead();
      }
      if (events[i].events & EPOLLOUT) {
        handler->handleWrite();
      }
#endif
    }
  }
}
