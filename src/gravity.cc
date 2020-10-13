#include "gravity_compiler.h"
#include "gravity_core.h"
#include "gravity_macros.h"
#include "gravity_vm.h"
#include "gravity_vmmacros.h"
#include <cassert>
#include <filesystem>
#include <iostream>

using namespace std;

static void report_error(gravity_vm *vm,
                         error_type_t error_type,
                         const char *description,
                         error_desc_t error_desc,
                         void *xdata) {
  cerr << "\t";
  switch(error_type) {
  case GRAVITY_ERROR_NONE: cerr << "NONE"; break;
  case GRAVITY_ERROR_SYNTAX: cerr << "SYNTAX"; break;
  case GRAVITY_ERROR_SEMANTIC: cerr << "SEMANTIC"; break;
  case GRAVITY_ERROR_RUNTIME: cerr << "RUNTIME"; break;
  case GRAVITY_WARNING: cerr << "WARNING"; break;
  case GRAVITY_ERROR_IO: cerr << "I/O"; break;
  }
  cerr << " ERROR on " << error_desc.fileid << "(" << error_desc.lineno << ", "
       << error_desc.colno << "): " << description << endl;
  exit(EXIT_FAILURE);
}

void compile(gravity_vm *vm, gravity_delegate_t delegate, const char *src) {
  shared_ptr<gravity_compiler_t> compiler(gravity_compiler_create(&delegate),
                                          [](gravity_compiler_t *c) {
                                            gravity_compiler_free(c);
                                          });

  gravity_closure_t *closure = gravity_compiler_run(compiler.get(),
                                                    src, strlen(src),
                                                    0, true, true);
  if (!closure) {
    cerr << "Couldn't compile code:\n" << src << endl;
    exit(EXIT_FAILURE);
  }
  gravity_compiler_transfer(compiler.get(), vm);
  gravity_vm_loadclosure(vm, closure);
}

void run(const char *src) {
  gravity_delegate_t delegate = {.error_callback = report_error};

  shared_ptr<gravity_vm> vm(gravity_vm_new(&delegate),
                            [](gravity_vm *v) {
                              gravity_vm_free(v);
                            });

  // tag::native[]
  gravity_vm_setvalue(vm.get(), "read", NEW_CLOSURE_VALUE([](gravity_vm *vm,
                                                       gravity_value_t *args,
                                                       uint16_t nargs,
                                                       uint32_t rindex) {
    RETURN_VALUE(VALUE_FROM_CSTRING(vm, "world"), rindex);
  }));
  // end::native[]

  compile(vm.get(), delegate, src);

  auto fn = gravity_vm_getvalue(vm.get(), "fn", strlen("fn"));
  if (!VALUE_ISA_CLOSURE(fn)) {
    cerr << "'fn' is no closure." << endl;
    exit(EXIT_FAILURE);
  }

  if (gravity_vm_runclosure(vm.get(), VALUE_AS_CLOSURE(fn), fn, nullptr, 0)) {
    auto result = gravity_vm_result(vm.get());
    auto value = string(VALUE_AS_CSTRING(result));
    assert(!value.compare("Hello, world"));
  } else {
    cerr << "Couldn't run 'fn'." << endl;
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[]) {
  shared_ptr<void> core(nullptr, [](void*) { gravity_core_free(); });

  auto src = "extern var read; func fn() { return \"Hello, \" + read(); }";
  run(src);

  cout << "| Gravity" << endl;
  cout << "| " << filesystem::file_size(argv[0]) << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
