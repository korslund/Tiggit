#include "jobqueue.hpp"

JobQueue* JobQueue::single = NULL;

JobQueue *JobQueue::get()
{
  if(single == NULL)
    single = new JobQueue;
  return single;
}

void JobQueue::doQueue(StatusJob *job)
{
  wxMutexLocker lock(mutex);
  list.push(job);
}

void JobQueue::doUnlink(StatusJob *job)
{
  wxMutexLocker lock(mutex);
  if(job == current)
    current = NULL;
}

void JobQueue::doUpdate()
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
