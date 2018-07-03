#pragma once

#include <google/protobuf/message.h>

namespace p2cache {

using MessagePtr = std::shared_ptr<google::protobuf::Message>;

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