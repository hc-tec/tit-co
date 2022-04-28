//
// Created by titto on 2022/4/28.
//

#ifndef TIT_COROUTINE_PROTOCOL_MANAGER_H
#define TIT_COROUTINE_PROTOCOL_MANAGER_H

#include <map>

#include "base/singleton.h"

#include "protocol.h"

namespace tit {

namespace co {


struct SrvTraits {
 public:

  virtual SerializerInterface* req_serializer() {
    return new Serializer<ProtocolInterface>;
  }

  virtual SerializerInterface* res_serializer() {
    return new Serializer<ProtocolInterface>;
  }

};

class ProtocolManager {
 public:
  void Register(const std::string& srv_name, SrvTraits* traits) {
    if (map_.find(srv_name) == map_.end()) {
      map_[srv_name] = traits;
    }
  }

  SrvTraits* Get(const std::string& srv_name) {
    auto pare = map_.find(srv_name);
    if (pare != map_.end()) {
      return pare->second;
    }
    return new SrvTraits();
  }

 private:
  std::map<std::string, SrvTraits*> map_;
};


using ProtocolManagerSingleton = base::Singleton<ProtocolManager>;


}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_PROTOCOL_MANAGER_H
