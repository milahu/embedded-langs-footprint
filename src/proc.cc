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
