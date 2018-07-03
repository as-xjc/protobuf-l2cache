#pragma once

#include <boost/utility/string_view.hpp>
#include <p2cache/result.hpp>

namespace p2cache {

class L1CacheIf {
 public:
  virtual Result Get(boost::string_view key) = 0;
  virtual void Set(boost::string_view key, MessagePtr data, int expire) = 0;
  virtual void Set(boost::string_view key, MessagePtr data) = 0;
  virtual void Del(boost::string_view key) = 0;
  virtual void RefreshExpired(boost::string_view key) = 0;
  virtual void Heartbeat() = 0;

  virtual void SetDefaultExpire(int expire) final { expire_ = expire; }
  virtual int GetDefaultExpire() final { return expire_; }

 private:
  int expire_{60};
};

}
