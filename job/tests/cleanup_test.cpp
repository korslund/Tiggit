#include "job.hpp"
#include <iostream>
#include <stdexcept>

using namespace Jobify;
using namespace std;

struct MyJob : Job
{
  int i;

  MyJob(int _i) : Job(), i(_i) {}

  void doJob()
  {
    cout << "Job " << i << ": ";
    if(i == 1)
      {
        cout << "OK (normal execution)\n";
        setDone();
      }
    else if(i == 2)
      {
        cout << "ERROR! (using setError)\n";
        setError("We can't have that crap!");
      }
    else
      {
        cout << "EXCEPTION! (throw)\n";
        throw runtime_error("blablah");
      }
  }

  void cleanup()
  {
    cout << "Cleaning up " << i;
    if(info->isError()) cout << " (Error: " << info->message << ")\n";
    else cout << " (Success!)\n";
  }
};

int main()
{
  MyJob a(1), b(2), c(3);

  a.run();
  b.run();
  c.run();

  return 0;
}
