#pragma once

#include <p2cache/l1cacheif.hpp>

namespace p2cache {

class GeneralL1Cache : public L1CacheIf {
 public:
  GeneralL1Cache();
  virtual ~GeneralL1Cache();

  Result Get(const std::string& key) override;
  void Set(const std::string& key, MessagePtr data, int expire) override;
  void Set(const std::string& key, MessagePtr data) override;
  void Del(const std::string& key) override;
  void RefreshExpired(const std::string& key) override;
  void Heartbeat() override;

 protected:
  struct DataInfo {
    MessagePtr data;
    std::time_t createTime{0};
    int expired{0};
  };
  std::map<std::string, std::shared_ptr<DataInfo>> cache_;
};

}

