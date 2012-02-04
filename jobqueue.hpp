#ifndef __JOBQUEUE_HPP_
#define __JOBQUEUE_HPP_

#include "jobify.hpp"
#include <queue>

struct JobQueue
{
  JobQueue() : current(NULL) {}

  void queue(StatusJob *job)
  {
    wxMutexLocker lock(mutex);
    list.push(job);
  }

  // Remove reference to a running job.
  void unlink(StatusJob *job)
  {
    wxMutexLocker lock(mutex);
    if(job == current)
      current = NULL;
  }

  // This needs to be called regularly (from the main thread) to
  // process queued jobs.
  void update()
  {
    wxMutexLocker lock(mutex);

    // Abort if there's already a job running
    if(current && !current->isFinished())
      return;

    current = NULL;

    // Are there any more jobs we could do?
    while(!list.empty())
      {
        // Yup. Fetch the next task.
        StatusJob *job = list.front();
        list.pop();

        // Has this job been aborted already?
        if(job->abortRequested())
          // Seems so. Try the next.
          continue;

        // Nope, we're good to go!
        current = job;
        job->run();

        // Jobs may finish immediately
        if(job->isFinished())
          continue;

        break;
      }
  }

private:
  std::queue<StatusJob*> list;
  StatusJob *current;

  wxMutex mutex;
};

// More dodgy global variables, yay!
JobQueue jobQueue;

#endif
