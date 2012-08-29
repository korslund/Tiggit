#ifndef __TIGLIB_REPOLOCATOR_HPP_
#define __TIGLIB_REPOLOCATOR_HPP_

#include <string>

namespace TigLibInt
{
  /* Get stored path, if any. Returns "" if no path has been stored.
   */
  std::string getStoredPath();

  /* Find legacy repository loctions, if any was found. Otherwise returns "".

     TODO: Temporarily disabled
  */
  std::string findLegacyRepo();

  /* Upgrade a repository path from a legacy format, if necessary.
     Always call this before you start using a repository. Will change
     files in-place.

     Returns false if no upgrade was needed.

     TODO: Temporarily disabled
   */
  bool upgradeRepo(const std::string &where);

  /* Set stored path, will be retrieved the next time getStoredPath is
     called. May return false if the given path was not usable.
  */
  bool setStoredPath(const std::string &dir);

  /* Get OS-dependent default path. Mostly intended as a default value
     to present to the user when getStoredPath() returns nothing.
  */
  std::string getDefaultPath();
}

#endif
