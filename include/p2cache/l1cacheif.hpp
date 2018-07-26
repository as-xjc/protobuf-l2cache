#pragma once

#include <p2cache/result.hpp>

namespace p2cache {

class L1CacheIf {
 public:
  virtual Result Get(const std::string& key) = 0;
  virtual void Set(const std::string& key, MessagePtr data, int expire) = 0;
  virtual void Set(const std::string& key, MessagePtr data) = 0;
  virtual void Del(const std::string& key) = 0;
  virtual void RefreshExpired(const std::string& key) = 0;
  virtual void Heartbeat() = 0;

  virtual void SetDefaultExpire(int expire) final { expire_ = expire; }
  virtual int GetDefaultExpire() final { return expire_; }

 private:
  int expire_{60};
};

}
