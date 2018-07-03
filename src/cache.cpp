#include <p2cache/cache.hpp>
#include <google/protobuf/util/json_util.h>
#include <p2cache/generall1cache.hpp>

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

Result P2Cache::ForceGet(boost::string_view key, bool cache) {
  auto result = backendGet(key);
  if (!result.Ok()) return result;

  if (option_.enableCache && cache) {
    l1cache_->Set(key, result.data);
  }

  return result;
}

void P2Cache::Set(boost::string_view key, MessagePtr ptr) {
  if (option_.enableCache) {
    l1cache_->Set(key, ptr);
  }

  std::string data;
  pbToString(ptr, data);
  backend_->Set(key, data);
}

Result P2Cache::backendGet(boost::string_view key) {
  if (!backend_) return Result{nullptr, State::NO_BACKEND};

  auto reply = backend_->Get(key);
  if (reply.empty()) return Result{nullptr, State::EMPTY};

  return stringToPb(reply);
}

void P2Cache::Del(boost::string_view key) {
  l1cache_->Del(key);
  backend_->Del(key);
}

void P2Cache::DelCache(boost::string_view key) {
  l1cache_->Del(key);
}

void P2Cache::RefreshExpired(boost::string_view key) {
  l1cache_->RefreshExpired(key);
}

void P2Cache::Heartbeat() {
  l1cache_->Heartbeat();

  if (backend_) backend_->Heartbeat();
}

bool P2Cache::InCache(boost::string_view key) {
  auto result = l1cache_->Get(key);

  return result.Ok();
}

}