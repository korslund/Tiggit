#include "liveinfo.hpp"
#include "tasks/download.hpp"
#include "tasks/notify.hpp"
#include "tasks/fileop.hpp"
#include "tasks/install.hpp"
#include "job/thread.hpp"
#include <boost/filesystem.hpp>
#include "repo.hpp"
#include <assert.h>
#include "server_api.hpp"
#include "launcher/run.hpp"

namespace bs = boost::filesystem;
using namespace TigLib;
using namespace Tasks;

#include <iostream>
using namespace std;

// Job that installs a game, notifies the system on success, and
// cleans up on error.
struct InstallGameJob : NotifyTask
{
  std::string idname, urlname, dir;
  Repo *repo;
  bool killOnError;

  InstallGameJob(const std::string &_url, const std::string &_zip,
                 const std::string &_dir, const std::string &_idname,
                 const std::string &_urlname, Repo *_repo,
                 Jobify::JobInfoPtr _info)
    : NotifyTask(new InstallTask(_url, _zip, _dir, _info)),
      idname(_idname), urlname(_urlname), dir(_dir),
      repo(_repo), killOnError(true)
  {}

  void onSuccess()
  {
    // Notify the repo that we are done. This updates config and tells
    // the server to count a dounwload.
    if(info->isSuccess())
      repo->downloadFinished(idname, urlname);
  }

  void onError()
  {
    // Remove files on error
    if(dir != "" && killOnError)
      bs::remove_all(dir);
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

  // Ignore offline mode. If the user wants to try installing a game,
  // then go ahead and try it.

  InstallGameJob *job = new InstallGameJob
    (ent->tigInfo.url, repo->getPath("incoming/" + ent->idname),
     repo->getInstDir(ent->idname), ent->idname, ent->urlname,
     repo, installJob);

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
  repo->gameUninstalled(ent->idname, ent->urlname);
  installJob->reset();

  // Set up a deletion job
  FileOpTask *job = new FileOpTask();
  res = job->getInfo();
  job->del(repo->getInstDir(ent->idname));

  Jobify::Thread::run(job, async);
  return res;
}

void LiveInfo::launch()
{
  assert(isInstalled());

  // Figure out what to run
  bs::path exe = getInstallDir();
  exe /= ent->tigInfo.launch;

  // Use executable location as working directory
  bs::path work = exe.parent_path();

  Launcher::run(exe.string(), work.string());
}

// Download screenshot, then notify the callback
struct ScreenshotJob : NotifyTask
{
  ShotIsReady *cb;
  std::string file, idname;

  ScreenshotJob(const std::string &_url,
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

  // Skip in offline mode
  if(!repo->offline)
    {
      // Fetch then start a job
      std::string url = ServerAPI::screenshotURL(ent->urlname);
      ScreenshotJob *job = new ScreenshotJob(url, file.string(), ent->idname,
                                             cb, screenJob);

      Jobify::Thread::run(job, async);
      return screenJob;
    }
}
