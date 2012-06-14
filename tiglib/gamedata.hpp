#ifndef __TIGLIB_GAMEDATA_HPP_
#define __TIGLIB_GAMEDATA_HPP_

#include "gameinfo/tigloader.hpp"
#include "list/mainlist.hpp"

namespace TigLib
{
  struct GameData
  {
    List::MainList allList;
    GameInfo::TigLoader data;

    /* This will copy the complete game list over from the TigLoader
       into the allList, and update it. Only pointers are copied so
       this is very inexpensive.

       copyList() should be called whenever the game data has been
       loaded or updated.
     */
    void copyList();
  };
}

#endif
