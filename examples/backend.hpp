#pragma once

#include <map>
#include "backend.hpp"
#include <iostream>

class MapBack : public p2cache::BackendIf {
 public:
  MapBack() {}
  ~MapBack() {}

  std::string Get(boost::string_view key) override {
    auto it = data_.find(key.data());
    if (it == data_.end()) return "";

    return it->second;
  }
  void Set(boost::string_view key, boost::string_view data) override {
    data_[key.to_string()] = data.to_string();
    std::cout << "<backend> key:" << key.to_string() << std::endl;
    std::cout << "<backend> data:" << data.to_string() << std::endl;
    std::cout << "<backend> length:" << data.size() << std::endl;
  }
  void Del(boost::string_view key) override {
    data_.erase(key.to_string());
  }

  void Heartbeat() override {

  }

 private:
  std::map<std::string, std::string> data_;
};