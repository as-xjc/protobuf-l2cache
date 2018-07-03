#pragma once

#include <p2cache/define.hpp>

namespace p2cache {

struct Result {
  MessagePtr data;
  State state;
  template <typename T>
  std::shared_ptr<T> Get() {
    static_assert(std::is_base_of<google::protobuf::Message, T>::value, "only support protobuf::Message");
    return std::dynamic_pointer_cast<T>(data);
  }

  bool Ok() { return state == State::OK; }
};

}