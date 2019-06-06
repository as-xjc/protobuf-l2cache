#pragma once

#include <functional>
#include <google/protobuf/message.h>

namespace p2cache {

using MessagePtr = std::shared_ptr<google::protobuf::Message>;

using TypeFilter = std::function<bool(MessagePtr&)>;

enum class State {
  OK,
  EMPTY,
  NO_BACKEND,
  FOUND_NO_MESSAGE_TYPE,
  FOUND_NO_MESSAGE_PROTOTYPE,
  PARSE_ERROR,
  DATA_ERROR,
};

}