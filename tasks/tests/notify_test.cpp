#include "notify.hpp"

#include <iostream>
#include <stdexcept>

using namespace std;
using namespace Tasks;
using namespace Jobify;

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

struct TestNote : NotifyTask
{
  TestNote(int i)
    : NotifyTask(new MyJob(i))
  {}

  void onSuccess() { cout << "SUCCESS notified\n"; }
  void onError() { cout << "ERROR notified: " << info->message << "\n"; }
};

int main()
{
  TestNote a(1), b(2), c(3);

  a.run();
  b.run();
  c.run();

  return 0;
}
