#ifndef __TASKS_MULTI_HPP_
#define __TASKS_MULTI_HPP_

#include <job/job.hpp>
#include <list>

namespace Tasks
{
  /* A MultiTask performs several jobs in sequence.

     Tasks are added with add(), and all tasks must be added before
     the MultiTask has started running.

     Sub-tasks are executed one after another in the order they were
     added. They are all executed directly in the same thread as
     doJob() is running; there is no internal awareness of
     multithreading.

     If one sub-task fails, the entire task fails. The task only
     succeeds if when all the sub-tasks have succeeded.

     Job objects are deleted when the multitask is deleted.

     It is highly recommended that you use the specialiced JobInfo
     struct returned by makeInfo() - OR let MultiTask set one up
     automatically by omitting the constructor parameter. (You can
     then fetch the pointer using Job::getInfo().)

     If you do this, progress reports (through info->getCurrent() and
     getTotal()) will work, and aborting tasks will work. Progress
     numbers only represent the currently running task though, and
     will reset between tasks (so task 1 will go from 0% to 100%, then
     task 2 will start back at 0% and up to 100%, etc.)

     If you do NOT do this (but instead use a standard JobInfo),
     calling info->abort() will ONLY take effect BETWEEN sub-tasks,
     and the progress numbers will all be zero.
   */

  struct MultiTask : Jobify::Job
  {
    MultiTask(Jobify::JobInfoPtr info = Jobify::JobInfoPtr());
    ~MultiTask();

    void add(Jobify::Job *j) { joblist.push_back(j); }

    static Jobify::JobInfoPtr makeInfo();

  protected:
    void doJob();

  private:
    typedef std::list<Jobify::Job*> JList;
    JList joblist;
  };
}

#endif
