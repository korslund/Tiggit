#include "dircopy.hpp"
#include <boost/filesystem.hpp>

#include <iostream>
using namespace std;
using namespace DirCopy;

int main()
{
  boost::filesystem::remove_all("_tmp2");

  copy("test.sh", "_tmp/test.sh");
  move("_tmp/test.sh", "_tmp/dir/test.sh");
  copy("_tmp/", "_tmp/self");
  move("_tmp", "_tmp2");

  return 0;
}
