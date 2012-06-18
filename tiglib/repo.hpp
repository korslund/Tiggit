#ifndef __TIGLIB_REPO_HPP_
#define __TIGLIB_REPO_HPP_

#include "gamedata.hpp"
#include "misc/lockfile.hpp"

namespace TigLib
{
  class Repo
  {
    Misc::LockFile lock;
    std::string dir;
    TigLib::GameData data;
    std::string listFile, tigDir;

    void initRepo();

  public:
    Repo(const std::string &where="");
    std::string getPath(const std::string &fname);
    void fetchFiles();
    void loadData();
    List::ListBase &mainList() { return data.allList; }
  };
}
#endif
