#ifndef __TIGLIB_REPOLOCATOR_HPP_
#define __TIGLIB_REPOLOCATOR_HPP_

#include <string>

namespace TigLibInt
{
  /* Get stored path, if any. Returns "" if no path has been stored.
   */
  std::string getStoredPath();

  /* Find legacy repository loctions, if any was found. Otherwise returns "".
  */
  std::string findLegacyRepo();

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
