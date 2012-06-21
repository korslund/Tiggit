#ifndef __TASKS_NOTIFY_HPP_
#define __TASKS_NOTIFY_HPP_

#include <job/job.hpp>

namespace Tasks
{
  /* NotifyTask is meant to be subclassed, and is used for getting
     notifications when another task succeeds or fails.

     Pass along a job to the constructor, and run it as normal. The
     NotifyTask will share the other job's JobInfo.

     The destructor deletes the job.
   */
  struct NotifyTask : Jobify::Job
  {
    NotifyTask(Jobify::Job *j);
    ~NotifyTask() { delete other; }

  protected:
    // One of these are called when the job has finished
    virtual void onSuccess() {}
    virtual void onError() {}

  private:
    Jobify::Job *other;

    void doJob();
    void cleanup();
  };
}

#endif
