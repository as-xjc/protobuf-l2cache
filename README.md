# P2Cache

基于`protobuf`的二级缓存库，主要用于缓存`protobuf`的`Message`数据。

## 一级缓存

目前使用`std::map`作为一级缓存使用

## 二级缓存

主要通过继承`BackendIf`接口类，可以实现`redis`、`Leveldb`等后端的缓存

### 数据格式

|<mode>|<type length>|<type name>|<data>|

* mode: 1字节。主要标记`data`的内容格式：`B`二进制形式和`J`文本格式(`Json`)
* type length: 1字节。记录`type name`的长度。
* type name：`protobuf`的`message`的类型名
* data：`protobuf`的数据

## API

参考`include/p2cache/cache.hpp`

## TODO

* 内置`redis`后端实现
* 一级缓存内部修改为并发`map`
* 一级缓存提供多样的缓存机制:`LRU`等