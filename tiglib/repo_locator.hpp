#ifndef __TIGLIB_REPOLOCATOR_HPP_
#define __TIGLIB_REPOLOCATOR_HPP_

#include <string>

namespace TigLibInt
{
  /* Find repository. Will locate legacy repository loctions, if any
     was found. Returns "" if nothing was found.
  */
  std::string findRepo();

  /* Upgrade a repository path from a legacy format, if necessary.
     Always call this before you start using a repository. Will change
     files in-place.

     Returns false if no upgrade was needed.
   */
  bool upgradeRepo(const std::string &where);

  /* Set stored path, will be retrieved the next time findRepo is
     called. May return false if the given path was not usable.
  */
  bool setStoredPath(const std::string &dir);

  /* Get OS-dependent default path. Mostly intended as a default value
     to present to the user when findRepo() returns nothing.
  */
  std::string getDefaultPath();
}

#endif
