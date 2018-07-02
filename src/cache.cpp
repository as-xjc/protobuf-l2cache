#include <p2cache/cache.hpp>
#include <google/protobuf/util/json_util.h>

namespace p2cache {

P2Cache::P2Cache(const Option& option, std::shared_ptr<BackendIf> e) : option_(option), backend_(e) {}

P2Cache::~P2Cache() {}

bool P2Cache::pbToString(MessagePtr msg, std::string& cache) {
  std::string type = msg->GetTypeName();
  char mode = 'B';
  if (option_.useJson) mode = 'J';
  cache.append(&mode, 1);
  auto length = static_cast<char>(type.size());
  cache.append(&length, 1);
  cache.append(msg->GetTypeName());

  bool result = true;
  if (option_.useJson) {
    std::string json;
    google::protobuf::util::Status state = google::protobuf::util::MessageToJsonString(*msg, &json);
    if (state.ok()) {
      result = true;
      cache.append(json);
    } else {
      result = false;
    }
  } else {
    result = msg->AppendToString(&cache);
  }
  return result;
}

Result P2Cache::stringToPb(const std::string& data) {
  boost::string_view view(data);
  if (view.size() < 2) {
    return Result{nullptr, State::DATA_ERROR};
  }

  char mode = 'B';
  if (view.copy(&mode, 1) != 1) {
    return Result{nullptr, State::DATA_ERROR};
  }
  view.remove_prefix(1);

  char length = 0;
  if (view.copy(&length, 1) != 1) {
    return Result{nullptr, State::DATA_ERROR};
  }
  view.remove_prefix(1);

  if (view.size() < length) {
    return Result{nullptr, State::DATA_ERROR};
  }

  auto type = view.substr(0, length).to_string();
  view.remove_prefix(length);
  auto msgDes = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type);
  if (msgDes == nullptr) {
    return Result{nullptr, State::FOUND_NO_MESSAGE_TYPE};
  }

  auto msg = google::protobuf::MessageFactory::generated_factory()->GetPrototype(msgDes);
  if (msg == nullptr) {
    return Result{nullptr, State::FOUND_NO_MESSAGE_PROTOTYPE};
  }

  MessagePtr ptr(msg->New());

  if (mode == 'B') {
    if (ptr->ParseFromArray(view.data(), view.size())) {
      return Result{ptr, State::OK};
    } else {
      return Result{nullptr, State::PARSE_ERROR};
    }
  } else if (mode == 'J') {
    std::string json = view.to_string();
    google::protobuf::util::Status state = google::protobuf::util::JsonStringToMessage(json, ptr.get());
    if (state.ok()) {
      return Result{ptr, State::OK};
    } else {
      return Result{nullptr, State::PARSE_ERROR};
    }
  }

  return Result{nullptr, State::PARSE_ERROR};
}

Result P2Cache::Get(boost::string_view key, bool copy) {
  auto result = cacheGet(key);
  if (result.state == State::OK) {
    if (copy) {
      MessagePtr ptr(result.data->New());
      ptr->CopyFrom(*result.data);
      result.data = ptr;
    }
    return result;
  }

  result = backendGet(key);
  if (result.state == State::OK && copy) {
    MessagePtr ptr(result.data->New());
    ptr->CopyFrom(*result.data);
    result.data = ptr;
  }

  return result;
}

Result P2Cache::ForceGet(boost::string_view key, bool cache) {
  auto result = backendGet(key);
  if (result.state != State::OK) return result;

  if (option_.enableCache && cache) {
    auto it = cache_.find(key.data());
    if (it != cache_.end()) {
      it->second->data = result.data;
    } else {
      auto info = std::make_shared<DataInfo>();
      info->data = result.data;
      info->expired = option_.defaultExpire;
      info->createTime = std::time(nullptr);
      cache_.emplace(key.to_string(), info);
    }
  }

  return result;
}

void P2Cache::Set(boost::string_view key, MessagePtr ptr) {
  if (option_.enableCache) {
    auto it = cache_.find(key.data());
    if (it != cache_.end()) {
      it->second->data = ptr;
    } else {
      auto info = std::make_shared<DataInfo>();
      info->data = ptr;
      info->expired = option_.defaultExpire;
      info->createTime = std::time(nullptr);
      cache_.emplace(key.to_string(), info);
    }
  }

  if (backend_) {
    std::string data;
    pbToString(ptr, data);
    backend_->Set(key, data);
  }
}

Result P2Cache::cacheGet(boost::string_view key) {
  if (!option_.enableCache) return Result{nullptr, State::EMPTY};

  auto it = cache_.find(key.data());
  if (it == cache_.end()) return Result{nullptr, State::EMPTY};

  auto now = std::time(nullptr);
  if (now - it->second->createTime >= it->second->expired) {
    cache_.erase(it);
    return Result{nullptr, State::EMPTY};
  }

  return Result{it->second->data, State::OK};
}

Result P2Cache::backendGet(boost::string_view key) {
  if (!backend_) return Result{nullptr, State::NO_BACKEND};

  auto reply = backend_->Get(key);
  if (reply.empty()) return Result{nullptr, State::EMPTY};

  return stringToPb(reply);
}

void P2Cache::Del(boost::string_view key) {
  if (option_.enableCache) {
    cache_.erase(key.data());
  }

  if (backend_) {
    backend_->Del(key);
  }
}

void P2Cache::DelCache(boost::string_view key) {
  if (option_.enableCache) {
    cache_.erase(key.data());
  }
}

void P2Cache::RefreshExpired(boost::string_view key) {
  auto it = cache_.find(key.data());
  if (it == cache_.end()) return;

  it->second->createTime = std::time(nullptr);
}

void P2Cache::Heartbeat() {
  auto now = std::time(nullptr);
  for (auto it = cache_.begin(); it != cache_.end(); ) {
    if (now - it->second->createTime >= it->second->expired) {
      it = cache_.erase(it);
    } else {
      ++it;
    }
  }

  if (backend_) backend_->Heartbeat();
}

bool P2Cache::InCache(boost::string_view key) {
  auto it = cache_.find(key.data());
  if (it == cache_.end()) return false;

  auto now = std::time(nullptr);
  if (now - it->second->createTime >= it->second->expired) {
    cache_.erase(it);
    return false;
  }

  return true;
}

}