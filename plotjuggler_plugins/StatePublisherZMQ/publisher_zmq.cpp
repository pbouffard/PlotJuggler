#include "publisher_zmq.h"

#include <cstdio>
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include <zmq.hpp>
#include "nlohmann/json.hpp"

std::vector<std::string> Tokenize(const std::string& str, const char delim) {
  std::vector<std::string> out;
  size_t start;
  size_t end = 0;

  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.emplace_back(str.substr(start, end - start));
  }

  return out;
}

std::pair<std::string, std::optional<int>> ArrayIndex(const std::string& str) {
  size_t bracket = str.find('[');
  if (bracket == std::string::npos || str.front() == '[' || str.back() != ']') {
    return {str, std::nullopt};
  }

  int index;
  std::istringstream s(str.substr(bracket + 1, str.size() - 1));
  if ((s >> index).fail()) {
    return {str, std::nullopt};
  }

  return {str.substr(0, bracket), index};
}

StatePublisherZMQ::StatePublisherZMQ() : context_(1) {
}

StatePublisherZMQ::~StatePublisherZMQ() {
}

void StatePublisherZMQ::setEnabled(bool enabled)
{
  enabled_ = enabled;
  if (enabled_) {
    socket_ = zmq::socket_t(context_, zmq::socket_type::pub);
    socket_.bind("tcp://*:5556");
  } else {
    socket_.close();
  }
}

nlohmann::json *GetItem(nlohmann::json& item, const std::string& full_name) {
  auto [name, index] = ArrayIndex(full_name);

  // Create empty array or object if it doesn't exist.
  if (!item.contains(name)) {
    if (index) {
      item[name] = nlohmann::json::array();
    } else {
      item[name] = nlohmann::json::object();
    }
  }

  nlohmann::json *ret = &item[name];
  if (index) {
    if (*index >= ret->size()) {
      ret->get_ptr<nlohmann::json::array_t*>()->resize(*index + 1);
    }
    return &(*ret)[*index];
  }

  return ret;
}

void StatePublisherZMQ::PublishData(double current_time) {
  if (!enabled_) {
    return;
  }

  using namespace std::chrono_literals;
  auto now = std::chrono::system_clock::now();
  auto delta = now - last_send_time_;
  if (delta < 17ms) {
    return;
  }
  last_send_time_ = now;

  nlohmann::json root;
  for (const auto& [full_name, data] : _datamap->numeric) {
    nlohmann::json *item = &root;

    auto tokens = Tokenize(full_name, '/');
    for (auto& token : tokens) {
      item = GetItem(*item, token);
    }

    std::optional<double> value = data.getYfromX(current_time);
    if (value) {
      *item = *value;
    } else {
      *item = nullptr;
    }
  }

  zmq::message_t msg(root.dump());
  socket_.send(msg, zmq::send_flags::none);
}
