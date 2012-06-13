#ifndef __TIGLIST_HPP_
#define __TIGLIST_HPP_

#include "tigentry.hpp"
#include <vector>

namespace TigData
{
  struct TigList
  {
    // General tiglist info
    std::string channel, desc, homepage;

    // Entries
    std::vector<TigEntry> list;
  };
}
#endif
