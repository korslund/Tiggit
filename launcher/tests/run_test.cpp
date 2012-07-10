#include "run.hpp"

#include <iostream>

using namespace std;
using namespace Launcher;

int main(int argc, char **argv)
{
  if(argc != 1)
    {
      cout << "Got arguments, exiting.\n";
      return 1;
    }

  cout << "Running ourselves\n";
  try { run("run_test"); }
  catch(exception &e)
    {
      cout << "CAUGHT: " << e.what() << endl;
    }
  catch(...)
    {
      cout << "An unknown error occured\n";
    }

  return 0;
}
