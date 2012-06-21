#include "liveinfo.hpp"
#include "tasks/download.hpp"
#include "tasks/unpack.hpp"
#include "job/thread.hpp"
#include <boost/filesystem.hpp>
#include "repo.hpp"
#include <assert.h>

namespace bs = boost::filesystem;
using namespace TigLib;

struct OtherInfo : Jobify::JobInfo
{
  Jobify::JobInfoPtr client;

  OtherInfo() : client(new Jobify::JobInfo) {}

  void abort() { doAbort = true; client->abort(); }

  int64_t getCurrent() { return current = client->getCurrent(); }
  int64_t getTotal() { return total = client->getTotal(); }
};

struct InstallJob : Jobify::Job
{
  std::string url, zip, dir, idname;
  Repo *repo;

  InstallJob(Jobify::JobInfoPtr _info)
    : Jobify::Job(_info) {}

  void doJob()
  {
    using namespace Jobify;

    // The JobInfo used by the sub tasks (download and install). We
    // keep this separate from our own because we don't want the
    // 'done' status of the download to imply that we are done
    // ourselves.
    JobInfoPtr client;

    {
      OtherInfo *inf = dynamic_cast<OtherInfo*>(info.get());
      assert(inf);
      client = inf->client;
    }

    assert(client && !client->isBusy());

    // Set up and run the download
    setBusy("Downloading");
    client->reset();
    {
      Tasks::DownloadTask dl(url, zip, client);
      dl.run();
    }

    // Abort on failure
    if(!client->isSuccess())
      {
        if(client->isError())
          setError(client->message);

        bs::remove(zip);
        return;
      }

    // Unpack
    setBusy("Unpacking");
    client->reset();
    {
      Tasks::UnpackTask unp(zip, dir, client);
      unp.run();
    }

    bs::remove(zip);

    if(!client->isSuccess())
      {
        if(client->isError())
          setError(client->message);

        bs::remove_all(dir);
        return;
      }

    // Everything is dandy!
    setDone();

    // Notify the config that we are done
    repo->setInstallStatus(idname, 2);
  }
};

struct UninstallJob : Jobify::Job
{
  std::string dir;

  UninstallJob(Jobify::JobInfoPtr _info)
    : Jobify::Job(_info) {}

  void doJob()
  {
    setBusy("Uninstalling");
    bs::remove_all(dir);
    setDone();
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
    installJob.reset(new OtherInfo);
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

  res.reset(new Jobify::JobInfo);
  UninstallJob *job = new UninstallJob(res);
  job->dir = repo->getInstDir(ent->idname);
  Jobify::Thread::run(job, async);
  return res;
}

void LiveInfo::launch(bool async)
{
  assert(isInstalled());

  // Invoke the launcher, which is an entirely separate module.
}

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

  Jobify::Thread::run(job, async);
  return screenJob;
}
