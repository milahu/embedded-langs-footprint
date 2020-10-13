#include <Ark/Ark.hpp>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
  Ark::State state(Ark::FeaturePersist);

  // tag::native[]
  state.loadFunction("read", [](vector<Ark::Value>&, Ark::VM*) {
    return Ark::Value("world");
  });
  // end::native[]

  auto src = "(let fn(fun() (+ \"Hello, \" (read))))";
  state.doString(src);

  Ark::VM vm(&state);
  vm.run();

  auto value = vm.call("fn").string().toString();
  assert(!value.compare("Hello, world"));

  cout << "| ArkScript" << endl;
  cout << "| " << filesystem::file_size(argv[0]) << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
