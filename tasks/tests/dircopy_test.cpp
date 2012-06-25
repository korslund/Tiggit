#include "dircopy.hpp"

#include <iostream>
using namespace std;
using namespace Tasks;
using namespace Jobify;

void testStatus(Job &j)
{
  JobInfoPtr info = j.getInfo();

  cout << "  ";
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
    DirCopyTask dir;
    dir.setDest("_dircopy0");
    testStatus(dir);
    testJob(dir);
  }

  {
    cout << "Copy one file, one dir:\n";
    DirCopyTask dir;
    dir.setDest("_dircopy1");
    dir.addSource("../tests/test.sh");
    dir.addSource("output/");
    testJob(dir);
  }

  {
    cout << "Copy dirs:\n";
    DirCopyTask dir;
    dir.setDest("_dircopy2");
    dir.addSource("./output");
    dir.addSource("../tests/_dircopy1/");
    testJob(dir);
  }

  {
    cout << "Move dirs:\n";
    DirCopyTask dir;
    dir.setDest("_dirmove", true);
    dir.addSource("_dircopy1");
    dir.addSource("_dircopy2");
    testJob(dir);
  }

  return 0;
}
