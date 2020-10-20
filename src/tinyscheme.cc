#include "proc.hh"
#include "scheme.h"
#include <cassert>
#include <iostream>
#include <memory>

using namespace std;

int main(int argc, char* argv[]) {
  shared_ptr<scheme> sc(scheme_init_new(), [](scheme *s) {
    scheme_deinit(s);
  });

  char buf[512] = {};
  scheme_set_output_port_string(sc.get(),
                                buf,
                                buf + sizeof(buf)/sizeof(char) - 1);

  static size_t rss;
  // tag::native[]
  scheme_registerable list[] = {
    {
      .f = [](scheme *sc, pointer args) -> pointer {
        rss = read_rss();
        return mk_string(sc, "world");
      },
      .name = "read"
    }};
  scheme_register_foreign_func_list(sc.get(), list, 1);
  // end::native[]

  auto src = "(define fn(lambda () (string-append \"Hello, \" (read))))";
  scheme_load_string(sc.get(), src);

  scheme_load_string(sc.get(), "(display (fn))");
  auto value = string(buf);
  assert(!value.compare("Hello, world"));

  cout << "| TinyScheme" << endl;
  cout << "| " << read_size(argv[0]) << endl;
  cout << "| " << rss << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
