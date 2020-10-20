#include "mem.hh"
#include <chaiscript/chaiscript.hpp>
#include <filesystem>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
  chaiscript::ChaiScript chai;

  static size_t rss;
  // tag::native[]
  chai.add(chaiscript::fun([]() -> string {
    rss = read_rss();
    return "world";
  }), "read");
  // end::native[]

  auto src = "def fn() { return \"Hello, \" + read(); }";
  chai(src);
  auto value = chai.eval<string>("fn()");
  assert(!value.compare("Hello, world"));

  cout << "| ChaiScript" << endl;
  cout << "| " << filesystem::file_size(argv[0]) / 1024 << endl;
  cout << "| " << rss << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
