#include "job.hpp"

#include <assert.h>
#include <exception>

using namespace Jobify;

void JobInfo::reset()
{
  current = total = 0;
  status = ST_NONE;
  doAbort = false;
  message = "";
}

void JobInfo::setError(const std::string &what)
{
  message = what;
  status = ST_ERROR;
}

bool JobInfo::checkStatus()
{
  // Check for abort requests
  if(doAbort) status = ST_ABORT;

  // Return true if any non-busy state has been set
  return !isBusy();
}

Job::Job(JobInfoPtr i) : info(i)
{
  assert(info);
  assert(!info->isCreated());
  info->reset();
  info->status = ST_CREATED;
}

void Job::run()
{
  assert(info->isCreated());
  assert(!info->isBusy());
  assert(!info->isFinished());
  if(info->doAbort)
    {
      setAbort();
      return;
    }
  setBusy();

  // Make sure we capture all exceptions, and flag them as job
  // failures.
  try
    { doJob(); }
  catch(std::exception &e)
    { setError(e.what()); }
  catch(...)
    { setError("Unknown error"); }

  assert(!info->isBusy());
}

void Job::setBusy(const std::string &what)
{
  info->message = what;
  info->status = ST_BUSY;
}

void Job::setAbort()
{
  assert(info->doAbort);
  info->status = ST_ABORT;
}
