#pragma once

#include <map>
#include "backend.hpp"
#include <iostream>

class MapBack : public p2cache::BackendIf {
 public:
  MapBack() = default;
  ~MapBack() = default;

  std::string Get(const std::string& key) override {
    auto it = data_.find(key);
    if (it == data_.end()) return "";

    return it->second;
  }
  void Set(const std::string& key, const std::string& data) override {
    data_[key] = data;
    std::cout << "<backend> key:" << key << std::endl;
    std::cout << "<backend> data:" << data << std::endl;
    std::cout << "<backend> length:" << data.size() << std::endl;
  }

  void Del(const std::string& key) override {
    data_.erase(key);
  }

  void Heartbeat() override {

  }

 private:
  std::map<std::string, std::string> data_;
};