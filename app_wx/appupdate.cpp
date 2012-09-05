#include "appupdate.hpp"

#include <spread/job/thread.hpp>
#include <spread/spread.hpp>

using namespace wxTigApp;
using namespace Spread;

struct UpdateJob : Job
{
  TigLib::Repo *repo;
  bool *hasNewUpdate;
  std::string *newExePath, *newVersion;

  void doJob()
  {
    bool &hasNew = *hasNewUpdate;
    std::string &newExe = *newExePath;
    std::string &newVer = *newVersion;

    hasNew = false;
    newExe = "";
    newVer = "";

    // Do the main data update first
    JobInfoPtr client = repo->fetchFiles();
    if(waitClient(client)) return;

    /* If there was no new update of the dataset, don't bother
       checking for a new app. Any new app release must necessarily be
       packaged as part of a data update.
    */
    if(repo->hasNewData())
      {
        std::string dest = repo->getPath("run");
        // TODO: Figure out where to put stuff
        client = repo->getSpread().install("tiggit.net", "wxtiggit-win32-1", dest);
        // TODO: Figure out how to recognize if there was an update,
        // then set appropriate values
        if(waitClient(client)) return;
      }

    setDone();
  }
};

Spread::JobInfoPtr AppUpdater::startJob()
{
  if(repo.offline) return JobInfoPtr();

  assert(!current || current->isFinished());

  UpdateJob *job = new UpdateJob;
  job->repo = &repo;
  job->hasNewUpdate = &hasNewUpdate;
  job->newExePath = &newExePath;
  job->newVersion = &newVersion;

  current = Thread::run(job);
  return current;
}

void AppUpdater::launchNew()
{
  assert(current && current->isSuccess());
  assert(hasNewUpdate);
  assert(newExePath != "");

  // TODO: do stuff here. Make sure we never restart to launch the
  // same app that's already running though.
}
