#include "notify.hpp"
#include <assert.h>

using namespace Tasks;
using namespace Jobify;

NotifyTask::NotifyTask(Jobify::Job *j)
  : other(j)
{
  assert(other);

  // This discards our own info struct, but that's OK.
  info = other->getInfo();
}

void NotifyTask::doJob()
{
  // Make sure we reset the info status before running the other
  // job, or it will complain that it has already started.
  assert(info->status == ST_BUSY);
  info->status = ST_CREATED;

  other->run();
  assert(!info->isBusy());
}

void NotifyTask::cleanup()
{
  if(info->isSuccess()) onSuccess();
  else onError();
}
