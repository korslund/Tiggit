#include "tiglib/repo.hpp"

namespace wxTigApp
{
  struct AppUpdater
  {
    bool hasNewUpdate;
    std::string newExePath, newVersion;

    AppUpdater(TigLib::Repo &_repo) : repo(_repo) {}

    /* Start a background update of all the base data: game info,
       screenshots, and the application itself. The function calls
       Repo::fetchFiles() as part of the process.

       Only the files on disk are updated, any data loaded in memory
       is not.
     */
    Spread::JobInfoPtr startJob();

    /* Launch the newly installed version, and close the current
       process. Only call this when the job started by startJob() has
       finished successfully, and hasNewUpdate is true.
     */
    void launchNew();

  private:
    TigLib::Repo &repo;
    Spread::JobInfoPtr current;
  };
}
