#pragma once

namespace p2cache {

struct Option {
  bool enableCache{true}; /**< 是否启用本地缓存，关闭则每次读写都是直接通过后端进行操作 */
  bool useJson{false}; /**< `protobuf`的数据是否使用`json`格式进行保存，默认为二进制模式 */
  uint32_t defaultExpire{30}; /**< 默认过期秒数 */
};

}
