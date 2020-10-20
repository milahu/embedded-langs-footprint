#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace std;

size_t read_rss() {
  stringstream ss;
  ss << "/proc/" << getpid() << "/statm";
  ifstream statm(ss.str());
  statm.ignore(256, ' ');
  size_t rss;
  statm >> rss;
  return rss * getpagesize() / 1024;
}
