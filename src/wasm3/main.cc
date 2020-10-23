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

  int block_new() {
    auto id = this->blocks.size();
    blocks[id] = make_shared<vector<int>>();
    return id;
  }

  int block_pushback(int id, int v) {
    auto ptr = blocks[id];
    ptr->push_back(v);
    return id;
  }

  int block_size(int id) {
    auto ptr = blocks[id];
    return ptr->size();
  }

  int block_read(int id, int idx) {
    auto ptr = blocks[id];
    return ptr->at(idx);
  }

  string as_str(int id) {
    auto ptr = blocks[id];
    return string(ptr->begin(), ptr->end());
  }

  int from_str(const string& str) {
    auto id = this->block_new();
    auto ptr = blocks[id];
    ptr->assign(str.begin(), str.end());
    return id;
  }

} mem;

int block_new() {
  return mem.block_new();
}

int block_pushback(int id, int v) {
  return mem.block_pushback(id, v);
}

int block_size(int id) {
  return mem.block_size(id);
}

int block_read(int id, int idx) {
  return mem.block_read(id, idx);
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
        auto runtime = env.new_runtime(2048);
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

        mod.link<block_new>("*", "block_new");
        mod.link<block_pushback>("*", "block_pushback");
        mod.link<block_size>("*", "block_size");
        mod.link<block_read>("*", "block_read");
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
