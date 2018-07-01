#pragma once

namespace p2cache {

struct Option {
  bool enableCache{true}; /**< 是否启用本地缓存，关闭则每次读写都是直接通过后端进行操作 */
  uint32_t defaultExpire{30}; /**< 默认过期秒数 */
};

}
