#ifndef __JOBIFY_THREAD_HPP
#define __JOBIFY_THREAD_HPP

#include "job.hpp"

namespace Jobify
{
  /* This black-box module swallows any Job object, executes it in a
     separate thread, then deletes the object after it returns. (The
     job must have been allocated using 'new'.)

     You can disable threading (run it locally) by setting
     async=false. This may seem pointless compared to just running
     j->run(), but it does provide a consistent interface for both
     cases, and it will also delete the Job before returning.
   */
  struct Thread
  {
    static void run(Job *j, bool async=true);
    static void sleep(double seconds);
  };
}
#endif
