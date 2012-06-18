#include "findrepo.hpp"
#include "misc/dirfinder.hpp"

static std::string getHomeDir()
{
  Misc::DirFinder find("tiggit.net", "tiggit");

  std::string res;
  if(find.getStoredPath(res))
    return res;

  if(!find.getStandardPath(res))
    return "";

  find.setStoredPath(res);
  return res;
}

std::string TigLib::findRepo()
{
  /* TODO: This function should be responsible for finding legacy
     repository locations. Kill getHomeDir and integrate the code from
     old/repo.hpp instead.
  */
  return getHomeDir();
}
