#pragma once

#include <boost/utility/string_view.hpp>

namespace pbcache {

class BackendIf {
 public:
  /**
 * 获取临时数据
 *
 * @note 当本地数据未命中，如果设置了redis，会从redis读取，并且放入cache，数据过期时间默认为10秒
 *
 * @param key 数据的key
 * @return 数据
 */
  virtual std::string Get(boost::string_view key) = 0;

  /**
   * 插入临时数据，同步到redis
   * @param key 数据的key
   * @param data 数据
   */
  virtual void Set(boost::string_view key, boost::string_view data) = 0;

  /// 删除临时数据
  virtual void Del(boost::string_view key) = 0;

  /// 心跳，用于驱动后端检测等事件
  virtual void Heartbeat() = 0;
};

}