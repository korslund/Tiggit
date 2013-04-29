#include "jobprogress.hpp"
#include <assert.h>

using namespace wxTigApp;

bool JobProgress::start(const std::string &msg)
{
  assert(info);
  setMsg(msg);

  // Poll-loop until the job finishes, or the user cancels it.
  while(true)
    {
      yield();
      wxMilliSleep(40);

      bool res;
      if(info->getTotal() != 0)
        {
          // Calculate progress
          int prog = (int)(info->getCurrent()*100.0/info->getTotal());
          // Avoid auto-closing at 100%
          if(prog >= 100) prog = 99;
          res = update(prog);
        }
      else
        res = pulse();

      // Did the user click 'Cancel'?
      if(!res)
        {
          // Abort
          info->abort();

          // Break here, don't rely on isFinished() because a hanged
          // job may never reach that state.
          break;
        }

      // If we're finished, exit.
      if(info->isFinished())
        break;
    }

  update(100);

  return info->isSuccess();
}
