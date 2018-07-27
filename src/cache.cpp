#include <p2cache/cache.hpp>
#include <boost/utility/string_view.hpp>
#include <google/protobuf/util/json_util.h>
#include <p2cache/generall1cache.hpp>

namespace {
const char DATA_TYPE_JSON = 'J';
const char DATA_TYPE_BINARY = 'B';
}

namespace p2cache {

P2Cache::P2Cache(const Option& option, std::unique_ptr<L1CacheIf> l1, std::unique_ptr<BackendIf> e) : option_(option) {
  l1cache_ = std::move(l1);
  backend_ = std::move(e);
}

P2Cache::P2Cache(const Option& option, std::unique_ptr<BackendIf> e) : option_(option) {
  l1cache_.reset(new GeneralL1Cache());
  l1cache_->SetDefaultExpire(option_.defaultExpire);
  backend_ = std::move(e);
}

P2Cache::P2Cache(const p2cache::Option& option) : option_(option) {
  l1cache_.reset(new GeneralL1Cache());
  l1cache_->SetDefaultExpire(option_.defaultExpire);
}

P2Cache::~P2Cache() {}

std::string P2Cache::pbToString(MessagePtr& msg) {
  std::string cache;
  const std::string type = msg->GetTypeName();

  char mode = DATA_TYPE_BINARY;
  if (option_.useJson) mode = DATA_TYPE_JSON;
  cache.append(&mode, 1);

  auto length = static_cast<char>(type.size());
  cache.append(&length, 1);
  cache.append(type);

  if (option_.useJson) {
    std::string json;
    auto state = google::protobuf::util::MessageToJsonString(*msg, &json);
    if (state.ok()) {
      cache.append(json);
      return cache;
    } else {
      return "";
    }
  } else {
    return msg->AppendToString(&cache) ? cache : "";
  }
}

Result P2Cache::stringToPb(const std::string& data) {
  boost::string_view view(data);
  if (view.size() < 2) {
    return Result{nullptr, State::DATA_ERROR};
  }

  char mode = DATA_TYPE_BINARY;
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

  if (mode == DATA_TYPE_BINARY) {
    if (ptr->ParseFromArray(view.data(), view.size())) {
      return Result{ptr, State::OK};
    } else {
      return Result{nullptr, State::PARSE_ERROR};
    }
  } else if (mode == DATA_TYPE_JSON) {
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

Result P2Cache::Get(const std::string& key, bool copy) {
  auto result = l1cache_->Get(key);

  if (!result.Ok()) {
    result = ForceGet(key, true);
  }

  if (result.state == State::OK && copy) {
    MessagePtr ptr(result.data->New());
    ptr->CopyFrom(*result.data);
    result.data = ptr;
  }

  return result;
}

Result P2Cache::ForceGet(const std::string& key, bool cache) {
  auto result = backendGet(key);
  if (!result.Ok()) return result;

  if (option_.enableCache && cache) {
    l1cache_->Set(key, result.data);
  }

  return result;
}

void P2Cache::Set(const std::string& key, MessagePtr ptr) {
  if (option_.enableCache) {
    l1cache_->Set(key, ptr);
  }

  if (backend_) {
    std::string data = pbToString(ptr);
    if (!data.empty()) {
      backend_->Set(key, data);
    }
  }
}

Result P2Cache::backendGet(const std::string& key) {
  if (!backend_) return Result{nullptr, State::NO_BACKEND};

  auto reply = backend_->Get(key);
  if (reply.empty()) return Result{nullptr, State::EMPTY};

  auto ptr = stringToPb(reply);
  if (!ptr.Ok()) return ptr;

  const std::string type = ptr.data->GetTypeName();
  const auto it = typefilters_.find(type);
  if (it != typefilters_.end() && !it->second(ptr.data)) {
    return Result{nullptr, State::EMPTY};
  }

  return ptr;
}

void P2Cache::Del(const std::string& key) {
  l1cache_->Del(key);
  if (backend_) backend_->Del(key);
}

void P2Cache::DelCache(const std::string& key) {
  l1cache_->Del(key);
}

void P2Cache::RefreshExpired(const std::string& key) {
  l1cache_->RefreshExpired(key);
}

void P2Cache::Heartbeat() {
  l1cache_->Heartbeat();

  if (backend_) backend_->Heartbeat();
}

bool P2Cache::InCache(const std::string& key) {
  auto result = l1cache_->Get(key);

  return result.Ok();
}

void P2Cache::AddTypeFilter(const std::string& type, TypeFilter filter) {
  typefilters_.emplace(type, std::move(filter));
}

void P2Cache::DelTypeFilter(const std::string& type) {
  typefilters_.erase(type);
}

}