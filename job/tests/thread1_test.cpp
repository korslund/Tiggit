#include "thread.hpp"
#include <iostream>
#include <unistd.h>

using namespace Jobify;
using namespace std;

struct MyJob : Job
{
  MyJob(JobInfoPtr j) : Job(j) {}

  void doJob()
  {
    setBusy("Sleeping for 5 seconds");
    info->total = 5;

    for(int i=0; i<5; i++)
      {
        info->current = i;
        Thread::sleep(1);
      }
    info->current = 5;
    setDone();
  }
};

int main()
{
  JobInfoPtr i(new JobInfo), i2(new JobInfo);

  Thread::run(new MyJob(i));
  Thread::run(new MyJob(i2));

  while(!i->isFinished())
    {
      Thread::sleep(0.3);
      cout << "Status: " << i->message << " (" << i->getCurrent() << "/" << i->getTotal() << ")\n";
    }

  return 0;
}
