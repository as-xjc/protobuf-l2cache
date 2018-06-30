#pragma once

#include <google/protobuf/message.h>
#include "state.hpp"

namespace pbcache {

using MessagePtr = std::shared_ptr<google::protobuf::Message>;

struct Result {
  MessagePtr data;
  State state;
  template <typename T>
  std::shared_ptr<T> Get() {
    static_assert(std::is_base_of<google::protobuf::Message, T>::value, "only support protobuf::Message");
    return std::dynamic_pointer_cast<T>(data);
  }
};

}