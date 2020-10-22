#include "proc.hh"
#include <cassert>
#include <cstdio>
#include <map>
#include <fstream>
#include "wasm3_cpp.h"

using namespace std;

class {
  map<int, shared_ptr<vector<int>>> blocks;

public:

  int new_block() {
    auto id = this->blocks.size();
    blocks[id] = make_shared<vector<int>>();
    return id;
  }

  int append_to_block(int id, int v) {
    auto ptr = blocks[id];
    ptr->push_back(v);
    return id;
  }

  int join_block(int dest, int src) {
    auto ptr_dest = blocks[dest];
    auto ptr_src = blocks[src];
    ptr_dest->insert(ptr_dest->end(), ptr_src->begin(), ptr_src->end());
    return dest;
  }

  string as_str(int id) {
    auto ptr = blocks[id];
    return string(ptr->begin(), ptr->end());
  }

  int from_str(const string& str) {
    auto id = this->new_block();
    auto ptr = blocks[id];
    ptr->assign(str.begin(), str.end());
    return id;
  }

} mem;

int new_block() {
  return mem.new_block();
}

int append_to_block(int id, int v) {
  return mem.append_to_block(id, v);
}

int join_block(int dest, int src) {
  return mem.join_block(dest, src);
}

static int rss;

// tag::declare[]
int read() {
  rss = read_rss();
  return mem.from_str("world");
}
// end::declare[]

int main(int argc, char* argv[]) {
    try {
        wasm3::environment env;
        auto runtime = env.new_runtime(1024);
        string fn("target/fn.wasm");
        if (argc > 1) {
          fn = argv[1];
        }
        ifstream file(fn, std::ios::binary);

        // it's the critical operation here: it turns off whitespace
        // skipping on formatted input streams even in binary mode.
        file >> std::noskipws;

        // Doesn't work, probably because missing `m_env(env)`
        // wasm3::module mod = env.parse_module(file);
        std::vector<uint8_t> buffer;

        std::copy(std::istream_iterator<uint8_t>(file),
                  std::istream_iterator<uint8_t>(),
                  std::back_inserter(buffer));

        auto mod = env.parse_module(buffer.data(), buffer.size());
        runtime.load(mod);

        mod.link<new_block>("*", "new_block");
        mod.link<append_to_block>("*", "append_to_block");
        mod.link<join_block>("*", "join");
        // tag::register[]
        mod.link<read>("*", "read");
        // end::register[]

        {
            auto test_fn = runtime.find_function("fn");
            auto res = test_fn.call<int>();
            auto value = mem.as_str(res);
            assert(!value.compare("Hello, world"));
        }

        cout << "| Wasm3" << endl;
        cout << "| " << read_size(argv[0]) << endl;
        cout << "| " << rss << endl;
        cout << "| N/A ^[1]^" << endl;

    } catch(wasm3::error &e) {
        cerr << "WASM3 error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
