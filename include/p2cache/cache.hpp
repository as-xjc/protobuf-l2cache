#pragma once

#include <map>
#include <string>
#include <memory>
#include <ctime>

#include <p2cache/backendif.hpp>
#include <p2cache/option.hpp>
#include <p2cache/define.hpp>
#include <p2cache/result.hpp>
#include <p2cache/l1cacheif.hpp>

namespace p2cache {

/**
 * 2级 protobuf 缓冲
 */
class P2Cache {
 public:
  P2Cache(const Option& option, std::unique_ptr<L1CacheIf> l1, std::unique_ptr<BackendIf> e);
  P2Cache(const Option& option, std::unique_ptr<BackendIf> e);
  P2Cache(const Option& option);
  virtual ~P2Cache();

  /**
   * 获取数据
   *
   * @note 当本地数据未命中，如果设置了`backend`，会从`backend`读取，并且放入cache
   *
   * @param key 数据的key
   * @return 数据
   */
  Result Get(const std::string& key, bool copy = false);

  /**
   * 插入数据，同步到`backend`
   * @param key 数据的key
   * @param data 数据
   */
  void Set(const std::string& key, MessagePtr data);

  /// 判断缓存中是否有数据
  bool InCache(const std::string& key);

  /// 删除数据，会删除`backend`的数据
  void Del(const std::string& key);

  /// 删除临时数据，只删除`cache`中的数据
  void DelCache(const std::string& key);

  /**
   * 强制读取数据，而是直接读取后端`backend`数据
   * @param key 数据的key
   * @param cache 是否更新`cache`中的数据
   * @return 读取到的数据
   */
  Result ForceGet(const std::string& key, bool cache = true);

  /// 刷新过期，以刷新那刻的时间重新计算超时
  void RefreshExpired(const std::string& key);

  /**
   * 提供给外部的心跳处理函数，让外面控制频率
   */
  void Heartbeat();

 private:
  std::string pbToString(MessagePtr&);
  Result stringToPb(const std::string& data);

  Result backendGet(const std::string& key);

  Option option_;

  std::unique_ptr<L1CacheIf> l1cache_;
  std::unique_ptr<BackendIf> backend_;
};

}