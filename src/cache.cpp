#include "cache.hpp"

namespace pbcache {

PbCache::PbCache(const Option& option, std::shared_ptr<BackendIf> e) : option_(option), backend_(e) {}

PbCache::~PbCache() {}

bool PbCache::pbToString(MessagePtr msg, std::string& cache) {
  cache.append(msg->GetTypeName());
  cache.append('\0', 1);

  return msg->AppendToString(&cache);
}

Result PbCache::stringToPb(const std::string& data) {
  boost::string_view view(data);

  auto pos = view.find_first_of("\0");
  if (pos == boost::string_view::npos) {
    return Result{nullptr, State::DATA_ERROR};
  }

  auto type = view.substr(0, pos).to_string();
  auto msgDes = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type);
  if (msgDes == nullptr) {
    return Result{nullptr, State::FOUND_NO_MESSAGE_TYPE};
  }

  auto msg = google::protobuf::MessageFactory::generated_factory()->GetPrototype(msgDes);
  if (msg == nullptr) {
    return Result{nullptr, State::FOUND_NO_MESSAGE_PROTOTYPE};
  }

  MessagePtr ptr(msg->New());
  view.remove_prefix(type.size() + 1);
  if (ptr->ParseFromArray(view.data(), view.size())) {
    return Result{ptr, State::OK};
  } else {
    return Result{nullptr, State::PARSE_ERROR};
  }
}

Result PbCache::Get(boost::string_view key) {
  auto result = cacheGet(key);
  if (result.state == State::OK) return result;

  return backendGet(key);
}

Result PbCache::ForceGet(boost::string_view key, bool cache) {
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

void PbCache::Set(boost::string_view key, MessagePtr ptr) {
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

Result PbCache::cacheGet(boost::string_view key) {
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

Result PbCache::backendGet(boost::string_view key) {
  if (!backend_) return Result{nullptr, State::NO_BACKEND};

  auto reply = backend_->Get(key);
  if (reply.empty()) return Result{nullptr, State::EMPTY};

  return stringToPb(reply);
}

void PbCache::Del(boost::string_view key) {
  if (option_.enableCache) {
    cache_.erase(key.data());
  }

  if (backend_) {
    backend_->Del(key);
  }
}

void PbCache::DelCache(boost::string_view key) {
  if (option_.enableCache) {
    cache_.erase(key.data());
  }
}

void PbCache::Heartbeat() {
  auto now = std::time(nullptr);
  for (auto it = cache_.begin(); it != cache_.end(); ) {
    if (now - it->second->createTime >= it->second->expired) {
      it = cache_.erase(it);
    } else {
      ++it;
    }
  }
}

}