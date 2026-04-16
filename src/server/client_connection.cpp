#include "client_connection.h"
#include "../event_loop/event_loop.h"
#include "request_handler.h"

#include <unistd.h>
#include <iostream>

ClientConnection::ClientConnection(int fd, EventLoop* loop, std::shared_ptr<RequestHandler> handler)
  : fd_(fd), loop_(loop), handler_(std::move(handler)) {
  // Automatically register for read events upon creation.
  loop_->addReadEvent(fd_, this);
}

ClientConnection::~ClientConnection() {
  loop_->removeAllEvents(fd_, this);
  close(fd_);
}

void ClientConnection::handleRead() {
  char buffer[1024];
  ssize_t bytes_read = read(fd_, buffer, sizeof(buffer));

  if (bytes_read > 0) {
    std::string request(buffer, bytes_read);
    std::string response = handler_->handle(request);
    
    // If the buffer was empty before this, we are not currently listening for write events.
    bool needs_write_event = write_buffer_.empty();
    write_buffer_ += response;

    if (needs_write_event && !write_buffer_.empty()) {
      loop_->addWriteEvent(fd_, this);
    }
  } else if (bytes_read == 0) {
    // Client disconnected gracefully.
    // Self-destruct logic: in a strict architecture a connection manager handles this,
    // but self-deletion is sufficient here since this is the end of the line for this handler.
    delete this;
  } else {
    // Read error (e.g., connection reset by peer).
    delete this;
  }
}

void ClientConnection::handleWrite() {
  if (write_buffer_.empty()) {
    loop_->removeWriteEvent(fd_, this);
    return;
  }

  ssize_t bytes_written = write(fd_, write_buffer_.c_str(), write_buffer_.size());
  if (bytes_written > 0) {
    write_buffer_.erase(0, bytes_written);
    if (write_buffer_.empty()) {
      loop_->removeWriteEvent(fd_, this);
    }
  } else if (bytes_written < 0) {
    // Write error
    delete this;
  }
}
