#ifndef __TIGLIB_LIVEINFO_HPP_
#define __TIGLIB_LIVEINFO_HPP_

#include "gameinfo/tigentry.hpp"

namespace TigLib
{
  /* This represents the 'live' counterpart to the game information in
     gameinfo/.

     Unlike TigData::TigEntry, which contains static, pre-loaded data,
     this structure is used to host dynamically updated information
     and functionality such as the local user's install status, rating
     information and fetching screenshot information.
   */
  struct LiveInfo
  {
    // Link to static game info
    const TigData::TigEntry *ent;

    // Not done
  };
}

#endif
