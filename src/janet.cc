#include <cassert>
#include <filesystem>
#include <iostream>
#include <janet.h>

using namespace std;

int main(int argc, const char *argv[]) {
  shared_ptr<void> janet([]() {
    janet_init();
    return nullptr;
  }(), [](void*) {
    janet_deinit();
  });

  auto env = janet_core_env(nullptr);

  // tag::native[]
  static const JanetReg cfuns[] = {{
      "read", [](int32_t argc, Janet *argv) -> Janet {
        return janet_cstringv("world");
      }, nullptr }};
  janet_cfuns(env, nullptr, cfuns);
  // end::native[]

  auto src = "(defn fn[] (string \"Hello, \" (read)))";
  auto status = janet_dostring(env, src, nullptr, nullptr);
  if (status != 0) {
    cerr << "Couldn't compile code:\n" << src << endl;
    return EXIT_FAILURE;
  }

  auto fn = janet_resolve_core("fn");
  if (janet_checktype(fn, JANET_NIL)) {
    cerr << "Couldn't find `fn`" << endl;
    return EXIT_FAILURE;
  }

  auto fiber = janet_fiber(janet_unwrap_function(fn), 64, 0, nullptr);
  fiber->env = env;
  auto out = janet_wrap_nil();
  janet_continue(fiber, janet_wrap_nil(), &out);

  if (janet_checktype(out, JANET_NIL)) {
    cerr << "Couldn't run `fn`" << endl;
    return EXIT_FAILURE;
  }

  auto value = string((const char *)janet_to_string(out));
  assert(!value.compare("Hello, world"));

  cout << "| Janet" << endl;
  cout << "| " << filesystem::file_size(argv[0]) << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
