#ifndef __TIGLIB_LIVEINFO_HPP_
#define __TIGLIB_LIVEINFO_HPP_

#include "gameinfo/tigentry.hpp"
#include "job/job.hpp"

namespace TigLib
{
  /* Callback functor called when a screenshot download is ready. It
     may either be invoked immediately from requestShot(), or from a
     worker thread. For asynchronous requests, make sure the reciever
     is thread safe!
   */
  struct ShotIsReady
  {
    virtual void shotIsReady(const std::string &idname,
                             const std::string &file) = 0;
  };

  /* This represents the 'live' counterpart to the game information in
     gameinfo/.

     Unlike TigData::TigEntry, which contains static, pre-loaded data,
     this structure is used to host dynamically updated information
     such as the local user's install status and rating information,
     and functionality such as screenshot fetching.
   */
  class Repo;
  struct LiveInfo
  {
    LiveInfo(const TigData::TigEntry *e, Repo *_repo);

    // Link to static game info
    const TigData::TigEntry *ent;

    // Extra data pointer, may be used for application-specific
    // user-data.
    void *extra;

    bool isInstalled() const;
    bool isUninstalled() const;
    bool isWorking() const;

    // Convenience function to get install status
    std::string progress(int64_t &current, int64_t &total)
    {
      if(installJob)
        {
          current = installJob->getCurrent();
          total = installJob->getTotal();
          return installJob->message;
        }
      current = total = 0;
      return "Inactive";
    }

    // Get direct access to the install job info status. May return an
    // uninitialized shared_ptr if no job is active.
    Jobify::JobInfoPtr getStatus() const { return installJob; }

    /* Install game into the repository. Returns the JobInfo
       associated with the installer job.

       If async is false, the function will not return until the
       install is complete.
     */
    Jobify::JobInfoPtr install(bool async = true);

    /* Uninstall game, or abort an install in progress.

       The returned JobInfo is local for the uninstall task only. It
       is NOT the same JobInfo returned by getStatus(), and will not
       affect overall install status for the game. The game (and the
       getStatus() return, and the isInstalled() etc functions) will
       all be marked as 'uninstalled' immediately.

       If async is false, the game will be uninstalled before the
       function returns, and the returned pointer will be empty.
     */
    Jobify::JobInfoPtr uninstall(bool async = true);

    // Send signal to abort current install job, if any.
    void abort() { if(isWorking()) installJob->abort(); }

    /* Launch this game.

       If async is true, launch a separate process and return
       immediately. If false, wait until it finishes.
     */
    void launch(bool async = true);

    // Mark this game as installed. Called on installed games at
    // startup.
    void markAsInstalled();

    // Set to true if this game was added since our last repo load.
    // Allows clients to notify users about newly added games.
    bool isNew() const { return sNew; }

    /* Request a screenshot. Will invoke the given callback when the
       screenshot is ready.

       If async==true, a download will start in the background if a
       cached file is not available. A pointer to the attached JobInfo
       is returned. If async==false, the function will not return
       until the screenshot is available.

       In any case, you may use JobInfo::isBusy, ::isSuccess(),
       ::isError etc on the result to inquire about status. You can
       also call JobInfo::reset() after error if you want to try
       again.

       Calling the function multiple times may or may not invoke the
       callback multiple times. The behavior depends on thread status
       and is thus undefined. Once the cache file is downloaded and
       available though, the callback will ALWAYS be invoked
       immediately.
    */
    Jobify::JobInfoPtr requestShot(ShotIsReady*, bool async = true);

  private:
    Jobify::JobInfoPtr screenJob, installJob;
    Repo *repo;

    // 'new' status, set by constructor
    bool sNew;

    // Initialize the installJob pointer
    void setupInfo();
  };
}

#endif
