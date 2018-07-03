#pragma once

#include <p2cache/l1cacheif.hpp>

namespace p2cache {

class GeneralL1Cache : public L1CacheIf {
 public:
  GeneralL1Cache();
  virtual ~GeneralL1Cache();

  Result Get(boost::string_view key) override;
  void Set(boost::string_view key, MessagePtr data, int expire) override;
  void Set(boost::string_view key, MessagePtr data) override;
  void Del(boost::string_view key) override;
  void RefreshExpired(boost::string_view key) override;
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

