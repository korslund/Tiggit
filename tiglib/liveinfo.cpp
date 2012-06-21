#include "liveinfo.hpp"
#include "tasks/download.hpp"
#include "tasks/unpack.hpp"
#include "tasks/multi.hpp"
#include "tasks/notify.hpp"
#include "tasks/fileop.hpp"
#include "job/thread.hpp"
#include <boost/filesystem.hpp>
#include "repo.hpp"
#include <assert.h>

namespace bs = boost::filesystem;
using namespace TigLib;
using namespace Tasks;

struct InstallJob : MultiTask
{
  std::string url, zip, dir, idname, urlname;
  Repo *repo;

  InstallJob(Jobify::JobInfoPtr _info)
    : MultiTask(_info) {}

  void doJob()
  {
    add(new DownloadTask(url, zip));
    add(new UnpackTask(zip, dir));

    try
      { MultiTask::doJob(); }
    catch(std::exception &e)
      { setError(e.what()); }
    catch(...)
      { setError("Unknown error"); }

    // Notify the config that we are done
    if(info->isSuccess())
      repo->downloadFinished(idname, urlname);
  }

  void cleanup()
  {
    // Remove files on exit
    if(info->isError())
      bs::remove_all(dir);
    bs::remove(zip);
  }
};

LiveInfo::LiveInfo(const TigData::TigEntry *e, Repo *_repo)
  : ent(e), extra(NULL), repo(_repo), myRate(-2)
{
  // Mark newly added games as 'new'.
  sNew = ent->addTime > repo->getLastTime();
}

bool LiveInfo::isInstalled() const
{
  // We use JobInfo status to signal an installed game, even if it was
  // not installed in this session.
  return installJob && installJob->isSuccess();
}

bool LiveInfo::isUninstalled() const
{
  /* Three cases count as 'uninstalled':
     - the JobInfo struct has not been set up yet (!installJob)
     - it has been set up, but has not been attached to any job, or
       reset() was called (!isCreated())
     - a previous install attempt failed or was aborted
       (isNonSuccess())
   */
  return !installJob || !installJob->isCreated() || installJob->isNonSuccess();
}

bool LiveInfo::isWorking() const
{
  return installJob && installJob->isCreated() && !installJob->isFinished();
}

void LiveInfo::setupInfo()
{
  if(!installJob)
    installJob = MultiTask::makeInfo();
}

void LiveInfo::markAsInstalled()
{
  assert(isUninstalled());
  setupInfo();
  installJob->setDone();
}

std::string LiveInfo::getInstallDir()
{
  assert(isInstalled());
  return repo->getInstDir(ent->idname);
}

int LiveInfo::getMyRating()
{
  if(myRate == -2)
    myRate = repo->getRating(ent->idname);

  return myRate;
}

void LiveInfo::setMyRating(int i)
{
  assert(i >= 0 && i <= 5);

  getMyRating();
  if(myRate != -1)
    return;

  myRate = i;
  repo->setRating(ent->idname, ent->urlname, myRate);
}

Jobify::JobInfoPtr LiveInfo::install(bool async)
{
  if(!isUninstalled())
    return installJob;

  setupInfo();
  installJob->reset();

  InstallJob *job = new InstallJob(installJob);

  job->url = ent->tigInfo.url;
  job->zip = repo->getPath("incoming/" + ent->idname);
  job->dir = repo->getInstDir(ent->idname);
  job->idname = ent->idname;
  job->urlname = ent->urlname;
  job->repo = repo;

  Jobify::Thread::run(job, async);

  return installJob;
}

Jobify::JobInfoPtr LiveInfo::uninstall(bool async)
{
  Jobify::JobInfoPtr res;

  if(!isInstalled())
    {
      if(isWorking())
        abort();
      return res;
    }

  // Mark the game as uninstalled
  repo->setInstallStatus(ent->idname, 0);
  installJob->reset();

  // Set up a deletion job
  FileOpTask *job = new FileOpTask();
  res = job->getInfo();
  job->del(repo->getInstDir(ent->idname));

  Jobify::Thread::run(job, async);
  return res;
}

void LiveInfo::launch(bool async)
{
  assert(isInstalled());

  // Invoke the launcher, which is an entirely separate module.
}

// Download screenshot, then notify the callback
struct DownloadNotify : NotifyTask
{
  ShotIsReady *cb;
  std::string file, idname;

  DownloadNotify(const std::string &_url,
                 const std::string &_file,
                 const std::string &_idname,
                 ShotIsReady *_cb,
                 Jobify::JobInfoPtr _info)
    : NotifyTask(new DownloadTask(_url, _file, _info)),
      cb(_cb), file(_file), idname(_idname) {}

  void onSuccess()
  {
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

  // File does not exist, and no work has been done. Go fetch the
  // screenshot.

  // URL is hard-coded for now. We will soon replace individual
  // downloads with a mass package update, so it doesn't matter.
  std::string url = "http://tiggit.net/pics/300x260/" + ent->urlname + ".png";
  DownloadNotify *job = new DownloadNotify(url, file.string(), ent->idname,
                                           cb, screenJob);

  Jobify::Thread::run(job, async);
  return screenJob;
}
