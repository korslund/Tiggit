#include "auto.hpp"
#include <iostream>
#include <assert.h>
#include <exception>

using namespace std;
using namespace Unpack;

void test(const std::string &file)
{
  cout << "Unpacking "+file+":\n  ";
  AutoUnpack unp;
  try
    {
      unp.unpack(file, "_outdir3");
    }
  catch(std::exception &e)
    {
      cout << e.what() << endl;
      return;
    }
  cout << "OK\n";
}

int main()
{
  test("test.sh");
  test("doesn't exist");
  test("archives/test.zip");
  return 0;
}
