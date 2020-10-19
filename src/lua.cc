#include <cassert>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

using namespace std;

int main(int argc, char* argv[]) {
  char buff[256];
  int error;
  shared_ptr<lua_State> L(luaL_newstate(), [](lua_State *l) {
    lua_close(l);
  });

  luaL_openlibs(L.get());

  // tag::native[]
  lua_pushcfunction(L.get(), [](lua_State *l) -> int {
    lua_pushstring(l, "world");
    return 1;
  });
  lua_setglobal(L.get(), "read");
  // end::native[]

  auto src = "function fn() return \"Hello, \" .. read() end";
  luaL_dostring(L.get(), src);

  lua_getglobal(L.get(), "fn");
  if (lua_pcall(L.get(), 0, 1, 0) != 0) {
    cerr << "error running function `fn':" << lua_tostring(L.get(), -1);
  } else {
    /* retrieve result */
    if (!lua_isstring(L.get(), -1)) {
      cerr << "function `fn' must return a string";
    } else {
      auto value = string(lua_tostring(L.get(), -1));
      assert(!value.compare("Hello, world"));

      lua_pop(L.get(), 1);  /* pop returned value */
    }
  }

  cout << "| Lua" << endl;
  cout << "| " << filesystem::file_size(argv[0]) << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
