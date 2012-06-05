#ifndef __JOBIFY_THREAD_HPP
#define __JOBIFY_THREAD_HPP

#include "job.hpp"

namespace Jobify
{
  /* This black-box module swallows any Job object, executes it in a
     separate thread, then deletes the object after it returns.
   */
  struct Thread
  {
    static void run(Job *j);
    static void sleep(double seconds);
  };
}
#endif
