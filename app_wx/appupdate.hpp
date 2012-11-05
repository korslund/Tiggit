#ifndef __APPWX_APPUPDATE_HPP_
#define __APPWX_APPUPDATE_HPP_

#include "tiglib/repo.hpp"

namespace wxTigApp
{
  struct AppUpdater
  {
    bool hasNewUpdate;
    std::string newExePath, newVersion;

    AppUpdater(TigLib::Repo &_repo)
      : hasNewUpdate(false), repo(_repo) {}

    /* Start a background update of all the base data: game info,
       screenshots, and the application itself. The function calls
       Repo::fetchFiles() as part of the process.

       Only the files on disk are updated, any data loaded in memory
       is not.
     */
    Spread::JobInfoPtr startJob();

    /* Run this as startup to launch the newest exe. The initial exe
       that the user launches (through desktop icons and similar) may
       not be the most up-to-date one, in large part because such
       system-installed versions are not installed in a place where we
       have write access.

       Returns true if a new exe was launched, meaning the current
       instance should exit immediately.
     */
    bool launchCorrectExe();

    /* Launch the newly installed version, and close the current
       process. Only call this when the job started by startJob() has
       finished successfully, and hasNewUpdate is true.

       If it returns true, the new program was successfully launched,
       meaning that this instance should exit as quickly as possible.
     */
    bool launchNew();
    bool launchIfNew() { return hasNewUpdate && launchNew(); }

    /* Call this to add the current running exe, and all DLLs and
       other files in the same directory, to the Spread cache.

       Currently only works on Windows.
     */
    void cacheLocalExeDir();

  private:
    TigLib::Repo &repo;
    Spread::JobInfoPtr current;
    bool launch();
  };
}
#endif
