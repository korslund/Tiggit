#ifndef __JOBQUEUE_HPP_
#define __JOBQUEUE_HPP_

#include "jobify.hpp"
#include <queue>

class JobQueue
{
  JobQueue() : current(NULL) {}
  static JobQueue* single;
  static JobQueue *get();

  std::queue<StatusJob*> list;
  StatusJob *current;
  wxMutex mutex;

  void doQueue(StatusJob *job);
  void doUnlink(StatusJob *job);
  void doUpdate();

public:

  // Queue a new job
  static void queue(StatusJob *job) { get()->doQueue(job); }

  // Remove reference to a running job.
  static void unlink(StatusJob *job) { get()->doUnlink(job); }

  // This needs to be called regularly (from the main thread) to
  // process queued jobs.
  static void update() { get()->doUpdate(); }
};

#endif
