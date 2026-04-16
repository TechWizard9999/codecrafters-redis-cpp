#pragma once

#include <string>

class RequestHandler {
public:
  virtual ~RequestHandler() = default;
  virtual std::string handle(const std::string& request) = 0;
};
