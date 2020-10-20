#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

size_t read_statm(int pos) {
  stringstream ss;
  ss << "/proc/" << getpid() << "/statm";
  ifstream statm(ss.str());
  for (int i = 0; i < pos; ++i) {
    size_t ignored;
    statm >> ignored;
  }
  size_t value;
  statm >> value;
  return value * getpagesize() / 1024;
}

size_t read_rss() {
  return read_statm(1);
}

size_t read_size(const char *filename) {
  return filesystem::file_size(filename) / 1024;
}

extern unsigned char _binary_target_python_std_zip_start;
extern unsigned char _binary_target_python_std_zip_size;

void setup_py_std() {
  auto tmp = filesystem::temp_directory_path();
  auto filename = filesystem::path(tmp).append("python-std.zip");
  ofstream b_stream(filename, fstream::out | fstream::binary);
  if (!b_stream) {
    cerr << "Couldn't create " << filename << endl;
    exit(EXIT_FAILURE);
  }

  b_stream.write(reinterpret_cast<char*>(&_binary_target_python_std_zip_start),
                 reinterpret_cast<size_t>(&_binary_target_python_std_zip_size));
  if (!b_stream.good()) {
    cerr << "Couldn't write " << filename << endl;
    exit(EXIT_FAILURE);
  }

  stringstream ss;
  ss << "unzip -o -q " << filename << " -d " << tmp;
  system(ss.str().c_str());
  setenv("PYTHONPATH", tmp.c_str(), 1);
}
