#ifndef __TIGLIST_HPP_
#define __TIGLIST_HPP_

#include "tigentry.hpp"
#include <vector>

namespace TigData
{
  struct TigList
  {
    // Channel name
    std::string channel;

    // Entries
    std::vector<TigEntry> list;
  };
}
#endif
