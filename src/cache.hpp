#pragma once

#include <map>
#include <string>
#include <memory>
#include <ctime>
#include <boost/utility/string_view.hpp>

#include "backendif.hpp"
#include "option.hpp"
#include "state.hpp"
#include "result.hpp"

namespace pbcache {

/**
 * 2级 protobuf 缓冲
 */
class PbCache {
 public:
  PbCache(const Option& option, std::shared_ptr<BackendIf> e);
  virtual ~PbCache();

  /**
   * 获取临时数据
   *
   * @note 当本地数据未命中，如果设置了redis，会从redis读取，并且放入cache
   *
   * @param key 数据的key
   * @return 数据
   */
  Result Get(boost::string_view key);

  /**
   * 插入临时数据，同步到redis
   * @param key 数据的key
   * @param data 数据
   */
  void Set(boost::string_view key, MessagePtr ptr);

  /// 删除数据
  void Del(boost::string_view key);

  /// 删除临时数据
  void DelCache(boost::string_view key);

  /**
   * 强制读取数据，不优先读取cache的数据，而是直接读取后端redis数据
   * @param key 数据的key
   * @param cache 是否更新cache中的数据
   * @return 读取到的数据
   */
  Result ForceGet(boost::string_view key, bool cache = true);

  /**
   * 提供给外部的心跳处理函数，让外面控制频率
   */
  void Heartbeat();

 private:
  bool pbToString(MessagePtr, std::string& cache);
  Result stringToPb(const std::string& data);

  Result cacheGet(boost::string_view key);
  Result backendGet(boost::string_view key);

  struct DataInfo {
    MessagePtr data;
    std::time_t createTime{0};
    uint32_t expired{0};
  };
  std::map<std::string, std::shared_ptr<DataInfo>> cache_;
  Option option_;

  std::shared_ptr<BackendIf> backend_;
};

}