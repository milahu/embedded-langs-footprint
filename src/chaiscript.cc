#include <chaiscript/chaiscript.hpp>
#include <filesystem>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
  chaiscript::ChaiScript chai;

  // tag::native[]
  chai.add(chaiscript::fun([]() -> string {
    return "world";
  }), "read");
  // end::native[]

  auto src = "def fn() { return \"Hello, \" + read(); }";
  chai(src);
  auto value = chai.eval<string>("fn()");
  assert(!value.compare("Hello, world"));

  cout << "| ChaiScript" << endl;
  cout << "| " << filesystem::file_size(argv[0]) << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
