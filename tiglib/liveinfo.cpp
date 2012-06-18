#include "liveinfo.hpp"
#include "tasks/download.hpp"
#include "job/thread.hpp"
#include <boost/filesystem.hpp>
#include "repo.hpp"

namespace bs = boost::filesystem;
using namespace TigLib;

// Custom job used to download, then notify a callback
struct DownloadNotify : Jobify::Job
{
  ShotIsReady *cb;
  std::string url, file, idname;

  DownloadNotify(Jobify::JobInfoPtr info)
    : Jobify::Job(info) {}

  void doJob()
  {
    // Reset info so we can reuse it
    info->reset();
    Tasks::DownloadTask dl(url, file, info);

    // Run job in this thread
    dl.run();

    // Abort on failure
    if(!info->isSuccess())
      return;

    // Inform the callback
    cb->shotIsReady(idname, file);
  }
};

Jobify::JobInfoPtr LiveInfo::requestShot(ShotIsReady *cb, bool async)
{
  if(!screenJob)
    screenJob.reset(new Jobify::JobInfo);

  // Don't interupt a working thread
  if(screenJob->isCreated() && !screenJob->isFinished())
    return screenJob;

  // If the job has already failed, don't try again.
  if(screenJob->isNonSuccess())
    return screenJob;

  // Calculate cache path
  bs::path file = repo->getPath("cache/shot300x260");
  file /= ent->idname + ".png";

  // Does the file already exist in cache?
  if(bs::exists(file))
    {
      cb->shotIsReady(ent->idname, file.string());
      screenJob->setDone();
      return screenJob;
    }

  // There should not be any active job at this point
  assert(!screenJob->isCreated());

  // File does not exist, and no work has been done. Create a new job.
  DownloadNotify *job = new DownloadNotify(screenJob);
  job->cb = cb;
  job->file = file.string();
  job->idname = ent->idname;
  // URL is hard-coded for now. We will soon replace individual
  // downloads with a mass package update, so it doesn't matter.
  job->url = "http://tiggit.net/pics/300x260/" + ent->urlname + ".png";

  if(async)
    Jobify::Thread::run(job);
  else
    {
      job->run();
      delete job;
    }
  return screenJob;
}
