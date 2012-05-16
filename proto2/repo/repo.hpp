#ifndef __REPO_HPP_
#define __REPO_HPP_

#include <tigdata/tiglist.hpp>
#include <map>

namespace Repo
{
  typedef std::map<std::string,TigList> ChanMap;

  struct Repo
  {
    ChanMap channels;

    std::string base_dir;

    void loadChannel(const std::string &name);
  };
}

#endif
