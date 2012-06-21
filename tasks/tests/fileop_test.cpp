#include "fileop.hpp"

#include <iostream>
using namespace std;
using namespace Tasks;
using namespace Jobify;

void testStatus(Job &j)
{
  JobInfoPtr info = j.getInfo();

  if(info->isBusy()) cout << "Busy!";
  else if(!info->hasStarted()) cout << "Not started yet!";
  else if(info->isSuccess()) cout << "Success!";
  else if(info->isNonSuccess()) cout << "Failure: " << info->message;
  cout << "  - progress " << info->getCurrent() << "/" << info->getTotal() << endl;
}

void testJob(Job &j)
{
  j.run();
  testStatus(j);
}

int main()
{
  {
    cout << "Empty test:\n";
    FileOpTask ops;
    testStatus(ops);
    testJob(ops);
  }

  {
    cout << "\nCopy:\n";
    FileOpTask ops;
    ops.copy("test.sh", "_tmp.out");
    testJob(ops);
  }

  {
    cout << "\nCopy - Remove - Move:\n";
    FileOpTask ops;
    ops.copy("_tmp.out", "_tmp2.out");
    ops.del("_tmp.out");
    ops.move("_tmp2.out", "_tmp3.out");
    testJob(ops);
  }

  {
    cout << "\nCopy to dir:\n";
    FileOpTask ops;
    ops.copy("_tmp3.out", "_outdir1/");
    testJob(ops);
  }

  return 0;
}
