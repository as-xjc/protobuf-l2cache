#pragma once

#include <string>

namespace p2cache {

class BackendIf {
 public:
  /**
 * 获取临时数据
 *
 * @note 当本地数据未命中，如果设置了`backend`，会从`backend`读取，并且放入`cache`
 *
 * @param key 数据的key
 * @return 数据
 */
  virtual std::string Get(const std::string& key) = 0;

  /**
   * 插入临时数据，同步到`backend`
   * @param key 数据的key
   * @param data 数据
   */
  virtual void Set(const std::string& key, const std::string& data) = 0;

  /// 删除临时数据
  virtual void Del(const std::string& key) = 0;

  /// 心跳，用于驱动后端检测等事件
  virtual void Heartbeat() = 0;
};

}