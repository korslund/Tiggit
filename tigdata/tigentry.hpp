#ifndef __TIGENTRY_HPP_
#define __TIGENTRY_HPP_

#include "tiginfo.hpp"
#include <stdint.h>

namespace TigData
{
  struct TigEntry
  {
    // Info from the stand-alone .tig file
    TigInfo tigInfo;

    // Time added to the channel database
    int64_t addTime;

    // Rating and download count info
    float rating;
    int rateCount, dlCount;

    // Idname = channel/urlname
    std::string urlname, channel, idname, tigurl;
  };
}
#endif
