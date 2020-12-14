#include "daScript/daScript.h"
#include "proc.hh"
#include <ostream>
#include <iostream>

using namespace das;
using namespace std;

static size_t rss;

// tag::native[]
class UserModule : public Module {
  static const char *read() {
    rss = read_rss();
    return "world";
  }

public:
  UserModule(): Module("usermod") {
    ModuleLibrary lib;
    lib.addModule(this);
    addExtern<DAS_BIND_FUN(read)>(*this, lib, "read", SideEffects::none, "read");
  }
};
// end::native[]

REGISTER_MODULE(UserModule);

int main(int argc, char* argv[]) {
  // request all da-script built in modules
  NEED_ALL_DEFAULT_MODULES;
  NEED_MODULE(UserModule);
  // make file access, introduce string as if it was a file
  auto fAccess = make_smart<FsFileAccess>();
  auto src = R""""(require usermod;[export] def fn { return "Hello, " + read(); })"""";
  auto fileInfo = make_unique<FileInfo>(src, uint32_t(strlen(src)));
  fAccess->setFileInfo("dummy.das", move(fileInfo));
  // compile script
  TextPrinter tout;
  ModuleGroup dummyLibGroup;
  auto program = compileDaScript("dummy.das", fAccess, tout, dummyLibGroup);
  // create context
  Context ctx(program->getContextStackSize());
  program->simulate(ctx, tout);
  // call context function
  auto value = string(cast<char *>::to(ctx.eval(ctx.findFunction("fn"), nullptr)));
  assert(!value.compare("Hello, world"));
  // shut-down daScript, free all memory
  Module::Shutdown();

  cout << "| daScript" << endl;
  cout << "| " << read_size(argv[0]) << endl;
  cout << "| " << rss << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
