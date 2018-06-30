#include "test.pb.h"
#include "cache.hpp"
#include "backend.hpp"

int main() {
  auto t= std::make_shared<test>();
  t->set_id(1);
  t->set_name("nihao");

  std::cout << t->DebugString() << std::endl;

  pbcache::Option op;
  auto end = std::make_shared<MapBack>();
  pbcache::PbCache cache(op, end);

  cache.Set("test", t);

  pbcache::Result result = cache.Get("test", true);
  std::cout <<"1:" <<result.Get<test>()->DebugString() << std::endl;
  auto p = result.Get<test>();
  p->set_id(2);
  cache.DelCache("test");
  std::cout <<"2:" <<cache.Get("test", true).Get<test>()->DebugString() << std::endl;


  return 0;
}

