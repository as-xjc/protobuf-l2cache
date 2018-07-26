#include <p2cache/generall1cache.hpp>

namespace p2cache {

GeneralL1Cache::GeneralL1Cache() {}

GeneralL1Cache::~GeneralL1Cache() {}

Result GeneralL1Cache::Get(const std::string& key) {
  auto it = cache_.find(key);
  if (it == cache_.end()) return Result{nullptr, State::EMPTY};

  auto now = std::time(nullptr);
  if (now - it->second->createTime >= it->second->expired) {
    cache_.erase(it);
    return Result{nullptr, State::EMPTY};
  }

  return Result{it->second->data, State::OK};
}

void GeneralL1Cache::Set(const std::string& key, MessagePtr data, int expire) {
  auto it = cache_.find(key);
  if (it != cache_.end()) {
    it->second->data = data;
  } else {
    auto info = std::make_shared<DataInfo>();
    info->data = data;
    info->expired = expire;
    info->createTime = std::time(nullptr);
    cache_.emplace(key, info);
  }
}

void GeneralL1Cache::Set(const std::string& key, MessagePtr data) {
  Set(key, data, GetDefaultExpire());
}

void GeneralL1Cache::Del(const std::string& key) {
  cache_.erase(key);
}

void GeneralL1Cache::RefreshExpired(const std::string& key) {
  auto it = cache_.find(key);
  if (it == cache_.end()) return;

  it->second->createTime = std::time(nullptr);
}

void GeneralL1Cache::Heartbeat() {
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