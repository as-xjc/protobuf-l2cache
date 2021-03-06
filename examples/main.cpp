#include "test.pb.h"
#include "p2cache/cache.hpp"
#include "backend.hpp"

int main() {
  auto t= std::make_shared<test>();
  t->set_id(1);
  t->set_name("nihao");

  std::cout << t->DebugString() << std::endl;

  p2cache::Option op;
  op.useJson = true;
  p2cache::P2Cache cache(op, std::unique_ptr<p2cache::BackendIf>(new MapBack));

  cache.AddTypeFilter(test::default_instance().GetTypeName(), [](p2cache::MessagePtr& ptr) {
    std::cout << "filer-----------:" << ptr->DebugString() << std::endl;
    return true;
  });
  cache.Set("test", t);

  p2cache::Result result = cache.Get("test", true);
  std::cout <<"1:" <<result.Get<test>()->DebugString() << std::endl;
  auto p = result.Get<test>();
  p->set_id(2);
  cache.DelCache("test");
  std::cout <<"2:" <<cache.Get("test", true).Get<test>()->DebugString() << std::endl;

  return 0;
}

