#include "thread.hpp"

#include <boost/thread.hpp>
#include <assert.h>

using namespace Jobify;

struct ThreadObj
{
  Job *j;
  void operator()()
  {
    j->run();
    assert(j->getInfo()->isFinished());
    delete j;
  }
};

void Thread::sleep(double seconds)
{
  int msecs = (int)(seconds*1000000);
  boost::this_thread::sleep(boost::posix_time::microseconds(msecs));
}

void Thread::run(Job *j)
{
  ThreadObj to;
  to.j = j;
  boost::thread trd(to);
}
