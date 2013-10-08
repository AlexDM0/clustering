#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <DBSCANModuleExt.h>

using namespace v8;
using namespace rur;

void RegisterModule(Handle<Object> exports) {
  DBSCANModuleExt::NodeRegister(exports);
}

NODE_MODULE(DBSCANModule, RegisterModule)
