#ifndef __TIGLIB_GAMEDATA_HPP_
#define __TIGLIB_GAMEDATA_HPP_

#include "gameinfo/tigloader.hpp"
#include "list/mainlist.hpp"
#include <map>

namespace TigLib
{
  struct LiveInfo;
  typedef std::map<std::string,LiveInfo*> InfoLookup;

  class Repo;
  struct GameData
  {
    List::MainList allList;
    GameInfo::TigLoader data;
    InfoLookup lookup;

    /* This will create a new set of LiveInfo structs representing all
       the TigEntrys in the tigloader list.

       Running this function includes deleting all the old LiveInfo
       structs and creating new ones. It should be called sparingly,
       and only when the main data has been reloaded.
     */
    void createLiveData(Repo *repo);

    // Kill all existing data
    void clear();

    LiveInfo *get(const std::string &id) const
    {
      InfoLookup::const_iterator it = lookup.find(id);
      if(it == lookup.end())
        return NULL;
      return it->second;
    }
  };
}

#endif
