#ifndef __TIGLIB_LIVEINFO_HPP_
#define __TIGLIB_LIVEINFO_HPP_

#include "gameinfo/tigentry.hpp"
#include <spread/job/jobinfo.hpp>

namespace TigLib
{
  /* This represents the 'live' counterpart to the game information in
     gameinfo/.

     Unlike TigData::TigEntry, which contains static, pre-loaded data,
     this structure is used to host dynamically updated information
     such as the local user's install status and rating information.
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

    // Total file size after install
    uint64_t instSize;

    /* Game version. May be empty.

       NOTE: This is the latest known package version. It might not
       correspond to the actual installed version.
     */
    std::string version;

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
          return installJob->getMessage();
        }
      current = total = 0;
      return "Inactive";
    }

    // Get direct access to the install job info status. May return an
    // uninitialized shared_ptr if no job is active.
    Spread::JobInfoPtr getStatus() const { return installJob; }

    /* Used to assign an existing install job to this game after a
       data reload. When data is reloaded from disk, all existing
       LiveInfo structures are deleted and new ones are created.
       However, you may want to keep around already running jobs, and
       reassign them to the corresponding new LiveInfos.
    */
    void setStatus(Spread::JobInfoPtr info) { installJob = info; }

    /* Install game into the repository. Returns the JobInfo
       associated with the installer job.

       If async is false, the function will not return until the
       install is complete.
     */
    Spread::JobInfoPtr install(bool async = true);

    /* Update the game to the latest version.

       TODO: This is not fully integrated into the rest of the system
       yet. A game that's being upgraded will show up (via.
       isInstalled etc) as just another installed game. This will be
       fixed later.
     */
    Spread::JobInfoPtr update(bool async = true);

    /* Uninstall game, or abort an install in progress.

       The returned JobInfo is local for the uninstall task only. It
       is NOT the same JobInfo returned by getStatus(), and will not
       affect overall install status for the game. The game (and the
       getStatus() return, and the isInstalled() etc functions) will
       all be marked as 'uninstalled' immediately.

       If async is false, the game will be uninstalled before the
       function returns, and the returned pointer will be empty.
     */
    Spread::JobInfoPtr uninstall(bool async = true);

    // Return install directory for this game. Only valid if the game
    // is installed.
    std::string getInstallDir() const;

    /* Get and set my rating for this game. Multiple ratings are
       currently ignored, but later the server may gain the capability
       to change a vote. Ratings are in the range 0-5, and
       getMyRating() returns -1 if no rating has been cast.
    */
    int getMyRating();
    void setMyRating(int i);

    // Send signal to abort current install job, if any.
    void abort() { if(isWorking()) installJob->abort(); }

    /* Launch this game. The game is started as a separate
       asynchronous process.
     */
    void launch() const;

    /* Mark this game as installed. Called on installed games at
       startup.

       If isUpdated (and optionally newVer) is set, then there is a
       new version available for this game. This is mostly cosmetic
       and won't change any behavior outside of what is shown to the
       user.
    */
    void markAsInstalled(const std::string &curVer, const std::string &newVer,
                         bool isUpdated);

    // Set to true if this game was added since our last repo load.
    // Allows clients to notify users about newly added games.
    bool isNew() const { return sNew; }

    /* Get the file name of the screenshot for this game. Returns an
       empty string if nothing was found.
    */
    std::string getScreenshot() const;

  private:
    Spread::JobInfoPtr installJob;
    Repo *repo;
    int myRate;

    // 'new' status, set by constructor
    bool sNew;

    // Initialize the installJob pointer
    void setupInfo();
  };
}

#endif
