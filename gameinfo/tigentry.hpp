#ifndef __TIGENTRY_HPP_
#define __TIGENTRY_HPP_

#include <stdint.h>
#include <string>
#include <vector>

namespace TigData
{
  // Tig entry flags. Meanings may change later on.
  enum TigFlags
    {
      TF_DEMO           = 0x01,
    };

  struct TigEntry
  {
    // Strings from the dataset
    std::string launch, title, desc, devname, homepage, tags, urlname;

    // Generated strings. idname = channel+"/"+urlname
    std::string channel, idname;

    // Required libraries. See app_wx/libraries.hpp for supported values.
    std::vector<std::string> libs;

    // See TigFlags for meaning.
    uint32_t flags;

    bool isDemo() const { return flags & TF_DEMO; }

    // Time added to the channel database
    uint64_t addTime;

    // Statistics info
    float rating;
    int rateCount, dlCount;

    TigEntry() : rating(-1), rateCount(0), dlCount(0) {}
  };
}
#endif
